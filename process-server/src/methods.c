#include "../include/db.h"
#include "../include/manager.h"
#include "../include/methods.h"
#include "../include/util.h"

#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// NOLINTNEXTLINE(modernize-macro-to-enum) : Macro is fine.
#define CONTENT_LENGTH_MAX_DIGITS 32 /** The maximum number of digits acceptable for the content size. */

/**
 * http_get
 * <p>
 * Handle an HTTP GET Request and generate the information necessary to assemble a Response.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param request the request
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @param entity_body pointer to the entity body for the response
 * @return 0 on success, -1 and set err on failure
 */
static int http_get(struct core_object *co, struct state_object *so, struct http_request *request,
                    size_t *status, struct http_header ***headers, char **entity_body);

static int fs_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *request,
                  size_t *status, struct http_header ***headers, char **entity_body);

static int db_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *request,
                  size_t *status, struct http_header ***headers, char **entity_body);

/**
 * http_head
 * <p>
 * Handle an HTTP HEAD Request and generate the information necessary to assemble a Response.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param request the request
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @param entity_body the entity body
 * @return 0 on success, -1 and set err on failure
 */
static int http_head(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body);

/**
 * http_post
 * <p>
 * Handle an HTTP POST Request and generate the information necessary to assemble a Response.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param request the request
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @param entity_body pointer to the entity body for the response
 * @return 0 on success, -1 and set err on failure
 */
static int http_post(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body);

/**
 * store_in_db
 * <p>
 * Store a request body in the database with the key as the URI.
 * </p>
 * @param co the core object
 * @param so the server object
 * @param uri the request URI
 * @param entity_body the entity body
 * @param entity_body_size the size of the entity body
 * @return 0 if the object was inserted, 1 if the object was updated, -1 and set err on failure.
 */
static int store_in_db(struct core_object *co, struct state_object *so, char *uri,
                       char *entity_body, size_t entity_body_size);

/**
 * store_in_fs
 * <p>
 * Store the entity body of a request in the file system at its request line.
 * </p>
 * @param co the core object
 * @param uri the uri at which to store the entity body
 * @param entity_body the entity body
 * @param entity_body_size the entity body size
 * @return 0 on success, -1 and set err on failure
 */
static int store_in_fs(struct core_object *co, char *uri, char *entity_body, size_t entity_body_size);

/**
 * post_assemble_response_innards
 * <p>
 * Assemble the innards of the Response to a POST Request.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @param entity_body pointer to the entity body for the response
 * @return 0 on success, -1 on failure
 */
static int post_assemble_response_innards(struct core_object *co, struct http_request *request,
                                          struct http_header ***headers, char **entity_body);

/**
 * get_assemble_response_innards
 * <p>
 * Assemble the innards of the Response to a GET Request.
 * </p>
 * @param co the core object
 * @param request the request object
 * @param headers pointer to the header list for the response
 * @param entity_body pointer to the entity body for the response
 * @return 0 on success, -1 on failure
 */
static int get_assemble_response_innards(off_t content_length, struct core_object *co, size_t *status,
                                         struct http_header ***headers);

int perform_method(struct core_object *co, struct state_object *so, struct http_request *request,
                   size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    char *method;
    
    method = request->request_line->method;
    
    if (strcmp(method, M_GET) == 0)
    {
        if (http_get(co, so, request, status, headers, entity_body) == -1)
        {
            return -1;
        }
    } else if (strcmp(method, M_HEAD) == 0)
    {
        if (http_head(co, so, request, status, headers, entity_body) == -1)
        {
            return -1;
        }
    } else if (strcmp(method, M_POST) == 0)
    {
        if (http_post(co, so, request, status, headers, entity_body) == -1)
        {
            return -1;
        }
    } else
    {
        *status      = NOT_IMPLEMENTED_501;
        *headers     = NULL;
        *entity_body = NULL;
    }
    
    return 0;
}

static int http_get(struct core_object *co, struct state_object *so, struct http_request *request,
                    size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    bool               db          = false;
    bool               conditional = false;
    struct http_header *database_header;
    
    database_header = get_header("database", request->extension_headers, request->num_extension_headers);
    if (database_header)
    {
        to_lower(database_header->value);
        db = strcmp(database_header->value, "true") == 0;
    }
    conditional     = get_header(H_IF_MODIFIED_SINCE, request->request_headers, request->num_request_headers) != NULL;
    
    if (db)
    {
        if (db_get(conditional, co, so, request, status, headers, entity_body) == -1)
        {
            return -1;
        }
    } else
    {
        if (fs_get(conditional, co, so, request, status, headers, entity_body) == -1)
        {
            return -1;
        }
    }
    return 0;
}

int fs_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *req,
           size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    char               pathname[BUFSIZ];
    struct stat        st;
    struct http_header *h;
    time_t             f_last_modified;
    time_t             h_last_modified;
    int                fd;
    
    memset(pathname, 0, BUFSIZ);
    if (getcwd(pathname, BUFSIZ) == NULL)
    {
        SET_ERROR(co->err);
        return -1;
    }
    strlcat(pathname, "/", BUFSIZ);
    strlcat(pathname, WRITE_DIR, BUFSIZ);
    strlcat(pathname, req->request_line->request_URI, BUFSIZ);
    
    
    // not found response
    if (stat(pathname, &st) == -1 || !S_ISREG(st.st_mode))
    {
        *status      = NOT_FOUND_404;
        *headers     = NULL;
        *entity_body = NULL;
        
        return 0;
    }
    
    if (conditional)
    {
        h = get_header(H_IF_MODIFIED_SINCE, req->request_headers, req->num_request_headers);
        if (!h)
        {
            (void) fprintf(stderr, "if-modified-since header not found in request\n");
            return -1;
        }
        f_last_modified = st.st_mtimespec.tv_sec;
        h_last_modified = http_time_to_time_t(h->value);
        if (h_last_modified == -1)
        {
            return -1;
        }
        if (difftime(f_last_modified, h_last_modified) < 0)
        {
            *status      = NOT_MODIFIED_304;
            *headers     = NULL;
            *entity_body = NULL;
            return 0;
        }
    }
    
    printf("OPEN FILE\n");
    fd = open(pathname, O_RDONLY);
    if (fd == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    *entity_body = mm_calloc(st.st_size + 1, sizeof(char), co->mm);
    if (!*entity_body)
    {
        SET_ERROR(co->err);
        return -1;
    }
    printf("READ FULLY\n");
    if (read_fully(fd, *entity_body, st.st_size) == -1)
    {
        return -1;
    }
    (*entity_body)[st.st_size] = '\0';
    printf("ASSEMBLE HEADERS\n");
    return get_assemble_response_innards(st.st_size, co, status, headers);
}

int db_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *req,
           size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    int                res;
    char               *path;
    char               d_last_modified_str[HTTP_TIME_LEN];
    time_t             d_last_modified;
    time_t             h_last_modified;
    struct http_header *h;
    char               *data;
    char               *value;
    datum              key;
    
    path = req->request_line->request_URI;
    key.dptr  = path;
    key.dsize = strlen(path) + 1;
    
    res = safe_dbm_fetch(co, DB_NAME, co->so->db_sem, &key, (uint8_t **) &data);
    if (res == -1)
    {
        return -1;
    }
    if (res == 1)
    {
        *status      = NOT_FOUND_404;
        *headers     = NULL;
        *entity_body = NULL;
        
        return 0;
    }
    strcpy(d_last_modified_str, data);
    value = data + strlen(data) + 1;
    
    if (conditional)
    {
        h = get_header(H_IF_MODIFIED_SINCE, req->request_headers, req->num_request_headers);
        if (!h)
        {
            (void) fprintf(stderr, "if-modified-since header not found in request\n");
            return -1;
        }
        d_last_modified = http_time_to_time_t(d_last_modified_str);
        h_last_modified = http_time_to_time_t(h->value);
        if (h_last_modified == -1)
        {
            return -1;
        }
        if (difftime(d_last_modified, h_last_modified) < 0)
        {
            *status      = NOT_MODIFIED_304;
            *headers     = NULL;
            *entity_body = NULL;
            return 0;
        }
    }
    
    *entity_body = mm_calloc(strlen(value) + 1, sizeof(char), co->mm);
    if (!*entity_body)
    {
        SET_ERROR(co->err);
        return -1;
    }
    strcpy(*entity_body, value);
    
    return get_assemble_response_innards((off_t) strlen(value), co, status, headers);
}

static int http_head(struct core_object *co, struct state_object *so, struct http_request *req,
                     size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    if (http_get(co, so, req, status, headers, entity_body) == -1)
    {
        return -1;
    }
    if (*entity_body) {
        if (mm_free(co->mm, *entity_body) == -1)
        {
            SET_ERROR(co->err);
            return -1;
        }
        *entity_body = NULL;
    }
    return 0;
}

static int http_post(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int                overwrite_status;
    struct http_header *database_header;
    struct http_header *content_length_header;
    size_t             entity_body_size;
    
    // Read headers to determine if database or file system
    database_header       = get_header("database", request->extension_headers, request->num_extension_headers);
    content_length_header = get_header(H_CONTENT_LENGTH, request->entity_headers, request->num_entity_headers);
    
    *entity_body = mm_strdup(request->entity_body, co->mm);
    
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers): Will never change
    entity_body_size = strtol(content_length_header->value, NULL, 10);
    
    // Store with key as URI
    if (database_header && strcmp(to_lower(database_header->value), "true") == 0)
    {
        overwrite_status = store_in_db(co, so, request->request_line->request_URI, *entity_body, entity_body_size);
    } else
    {
        overwrite_status = store_in_fs(co, request->request_line->request_URI, *entity_body, entity_body_size);
    }
    
    switch (overwrite_status)
    {
        case 0: // insert no overwrite
        {
            *status = CREATED_201;
            break;
        }
        case 1: // insert overwrite
        {
            *status = OK_200;
            break;
        }
        case -1: // error
        {
            return -1;
        }
        default:;
    }
    
    if (post_assemble_response_innards(co, request, headers, entity_body) == -1)
    {
        return -1;
    }
    
    return 0;
}

static int store_in_db(struct core_object *co, struct state_object *so, char *uri,
                       char *entity_body, size_t entity_body_size)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int     overwrite_status;
    char    timestamp[HTTP_TIME_LEN];
    size_t  timestamp_size;
    uint8_t *database_buffer;
    size_t  database_buffer_size;
    datum   key;
    datum   value;
    
    // Get the timestamp.
    if (http_time_now(timestamp) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    timestamp_size = strlen(timestamp) + 1;
    
    // Create a buffer for the database value.
    database_buffer_size = timestamp_size + entity_body_size + 1;
    database_buffer      = mm_malloc(database_buffer_size, co->mm);
    if (!database_buffer)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    // Put the timestamp\0entitybody\0 into the buffer.
    memcpy(database_buffer, timestamp, timestamp_size);
    memcpy(database_buffer + timestamp_size, entity_body, entity_body_size);
    *(database_buffer + database_buffer_size - 1) = '\0'; // Place /0 at end.
    
    key.dptr    = uri;
    key.dsize   = strlen(uri) + 1;
    value.dptr  = database_buffer;
    value.dsize = database_buffer_size;
    
    overwrite_status = db_upsert(co, DB_NAME, so->db_sem, &key, &value);
    
    mm_free(co->mm, database_buffer);
    
    return overwrite_status;
}

static int store_in_fs(struct core_object *co, char *uri, char *entity_body, size_t entity_body_size)
{
    PRINT_STACK_TRACE(co->tracer);
    
    char pathname[BUFSIZ];
    int  overwrite_status;
    
    memset(pathname, 0, BUFSIZ);
    if (getcwd(pathname, BUFSIZ) == NULL)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    *(pathname + strlen(pathname)) = '/';
    strlcat(pathname, WRITE_DIR, BUFSIZ);
    
    overwrite_status = write_to_dir(co, pathname, uri,
                                    entity_body, entity_body_size);
    
    return overwrite_status;
}

static int post_assemble_response_innards(struct core_object *co, struct http_request *request,
                                          struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    const int          num_headers = 2;
    struct http_header *content_type;
    struct http_header *content_length;
    char               entity_body_size[CONTENT_LENGTH_MAX_DIGITS];
    size_t             offset;
    
    content_type   = mm_malloc(sizeof(struct http_header), co->mm);
    content_length = mm_malloc(sizeof(struct http_header), co->mm);
    *headers = mm_malloc((num_headers + 1) * sizeof(struct http_header *), co->mm);
    if (!(content_type && content_length && *headers))
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    memset(entity_body_size, 0, CONTENT_LENGTH_MAX_DIGITS);
    if (sprintf(entity_body_size, "%lu", strlen(*entity_body)) == -1)
    {
        mm_free(co->mm, content_type);
        mm_free(co->mm, content_length);
        mm_free(co->mm, headers);
        SET_ERROR(co->err);
        return -1;
    }
    
    content_type   = set_header(co, H_CONTENT_TYPE, "text/html");
    content_length = set_header(co, H_CONTENT_LENGTH, entity_body_size);
    
    offset = 0;
    *(*headers + offset++) = content_type;
    *(*headers + offset++) = content_length;
    *(*headers + offset)   = NULL;
    
    return 0;
}

static int get_assemble_response_innards(off_t content_length, struct core_object *co, size_t *status,
                                         struct http_header ***headers)
{
    PRINT_STACK_TRACE(co->tracer);
    
    const int          num_headers = 2;
    struct http_header *h_content_type;
    struct http_header *h_content_length;
    char               content_length_str[H_CONTENT_LENGTH_LENGTH];
    
    h_content_type = set_header(co, H_CONTENT_TYPE, TEXT_HTML_CONTENT_TYPE);
    if (!h_content_type)
    {
        return -1;
    }
    
    if (sprintf(content_length_str, "%lld", content_length) < 0)
    {
        destroy_http_header(h_content_type, co);
        return -1;
    }
    h_content_length = set_header(co, H_CONTENT_LENGTH, content_length_str);
    if (!h_content_length)
    {
        destroy_http_header(h_content_type, co);
        return -1;
    }
    
    *headers = mm_calloc(num_headers + 1, sizeof(struct http_header), co->mm);
    if (!*headers)
    {
        destroy_http_header(h_content_type, co);
        destroy_http_header(h_content_type, co);
        SET_ERROR(co->err);
        return -1;
    }
    
    (*headers)[0] = h_content_length;
    (*headers)[1] = h_content_type;
    (*headers)[2] = NULL;
    
    *status = OK_200;
    
    return 0;
}
