#include "../include/db.h"
#include "../include/manager.h"
#include "../include/methods.h"
#include "../include/util.h"

#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

static int http_post(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body);

static int http_head(struct core_object *co, struct state_object *so, struct http_request *request, size_t *status,
                     struct http_header ***headers);

static int http_get(struct core_object *co, struct state_object *so, struct http_request *request,
                    size_t *status, struct http_header ***headers, char **entity_body);

static int fs_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *request,
                              size_t *status, struct http_header ***headers, char **entity_body);

static int db_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *request,
                              size_t *status, struct http_header ***headers, char **entity_body);

static int store_in_fs(struct core_object *co, const struct http_request *request);

static int post_insert_assemble_response_innards(struct core_object *co, struct http_request *request, size_t *status,
                                                 struct http_header ***headers, char **entity_body);

int perform_method(struct core_object *co, struct state_object *so, struct http_request *request,
                   size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    // tree on request line
    char *method;
    
    method = request->request_line->method;
    
    if (strcmp(method, "GET") == 0)
    {
        http_get(co, so, request, status, headers, entity_body);
    } else if (strcmp(method, "HEAD") == 0)
    {
        *entity_body = NULL;
        http_head(co, so, request, status, headers);
    } else if (strcmp(method, "POST") == 0)
    {
        http_post(co, so, request, status, headers, entity_body);
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
    bool db = false;
    bool conditional = false;
    struct http_header * database_header;

    database_header = get_header("database", request->extension_headers, request->num_extension_headers);
    to_lower(database_header->value);
    db = strcmp(database_header->value, "true") == 0;
    conditional = get_header(H_IF_MODIFIED_SINCE, request->request_headers, request->num_request_headers) != NULL;

    if (db) {
        if (db_get(conditional, co, so, request, status, headers, entity_body) == -1) {
            *status = INTERNAL_SERVER_ERROR_500;
            *headers     = NULL;
            *entity_body = NULL;
        }
    } else {
        if (fs_get(conditional, co, so, request, status, headers, entity_body) == -1) {
            *status = INTERNAL_SERVER_ERROR_500;
            *headers     = NULL;
            *entity_body = NULL;
        }
    }

    return 0;
}

int fs_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *req,
                       size_t *status, struct http_header ***headers, char **entity_body) {
    char pathname[BUFSIZ];
    struct stat st;
    struct http_header * h;
    time_t f_last_modified, h_last_modified;
    int fd;

    h = get_header(H_IF_MODIFIED_SINCE, req->request_headers, req->num_request_headers);
    if (!h) {
        (void) fprintf(stderr, "if-modified-since header not found in request\n");
        return -1;
    }

    memset(pathname, 0, BUFSIZ);
    if (getcwd(pathname, BUFSIZ) == NULL)
    {
        SET_ERROR(co->err);
        return -1;
    }
    strlcat(pathname, WRITE_DIR, BUFSIZ);
    strlcat(pathname, req->request_line->request_URI, BUFSIZ);

    // not found response
    if (stat(pathname, &st) == -1) {
        *status = NOT_FOUND_404;
        *headers     = NULL;
        *entity_body = NULL;

        return 0;
    }

    if (conditional) {
        f_last_modified = st.st_mtimespec.tv_sec;
        h_last_modified = http_time_to_time_t(h->value);
        if (h_last_modified == -1) {
            return -1;
        }
        if (difftime(f_last_modified, h_last_modified) < 0) {
            *status = NOT_MODIFIED_304;
            *headers     = NULL;
            *entity_body = NULL;
            return 0;
        }
    }

    fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        SET_ERROR(co->err);
        return -1;
    }
    *entity_body = mm_calloc(st.st_size, sizeof(char), co->mm);
    if (!*entity_body) {
        SET_ERROR(co->err);
        return -1;
    }
    if (read_fully(fd, *entity_body, st.st_size) == -1) {
        return -1;
    }

    // TODO: assemble 200 response innards
    return 0;
}

int db_get(bool conditional, struct core_object *co, struct state_object *so, struct http_request *request,
                       size_t *status, struct http_header ***headers, char **entity_body) {
    //TODO: this entire function
}

static int http_head(struct core_object *co, struct state_object *so, struct http_request *request, size_t *status,
                     struct http_header ***headers)
{
    PRINT_STACK_TRACE(co->tracer);
    
    return 0;
}

static int http_post(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    struct http_header *database_header;
    
    // Read headers to determine if database or file system
    // TODO: which one of these is it in?       lets do it in extension headers
    database_header = get_header("database", request->extension_headers, request->num_extension_headers);
    
    // Store with key as URI
    to_lower(database_header->value);
    if (strcmp(database_header->value, "true") == 0)
    {
        int upsert_status;
        
        upsert_status = db_upsert(co, so, request->request_line->request_URI,
                                  strlen(request->request_line->request_URI),
                                  request->entity_body, strlen(request->entity_body));
        
        switch (upsert_status)
        {
            case 0: // insert no overwrite
            {
                if (post_insert_assemble_response_innards(co, request, status, headers, entity_body) == -1)
                {
                    return -1;
                }
                break;
            }
            case 1: // insert overwrite
            {
                break;
            }
            case -1: // error
            {
                break;
            }
            default:;
        }
    } else
    {
        if (store_in_fs(co, request) == -1) // TODO: should indicate whether overwrite occurred
        {
            return -1;
        }
    }
    
    return 0;
}

static int post_insert_assemble_response_innards(struct core_object *co, struct http_request *request, size_t *status,
                                                 struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    const int num_headers = 2;
    
    struct http_header *content_type;
    struct http_header *content_length;
    
    *status      = OK_200;
    *entity_body = request->entity_body;
    
    content_type   = mm_malloc(sizeof(struct http_header), co->mm);
    content_length = mm_malloc(sizeof(struct http_header), co->mm);
    *headers = mm_malloc((num_headers + 1) * sizeof(struct http_header *), co->mm);
    if (!(content_type && content_length && *headers))
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    
    return 0;
}

static int store_in_fs(struct core_object *co, const struct http_request *request)
{
    char pathname[BUFSIZ];
    
    memset(pathname, 0, BUFSIZ);
    if (getcwd(pathname, BUFSIZ) == NULL)
    {
        SET_ERROR(co->err);
        return -1; // TODO: not necessarily fatal.
    }
    
    strlcat(pathname, WRITE_DIR, BUFSIZ);
    
    if (write_to_dir(pathname, request->request_line->request_URI,
                     request->entity_body, strlen(request->entity_body)) == -1)
    {
        SET_ERROR(co->err);
        return -1; // TODO: not necessarily fatal.
    }
    
    return 0;
}
