#include "../include/db.h"
#include "../include/manager.h"
#include "../include/methods.h"
#include "../include/util.h"

#include <unistd.h>
#include <stdlib.h>

#define CONTENT_LENGTH_MAX_DIGITS 32 /** The maximum number of digits acceptable for the content size. */

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
 * http_head
 * <p>
 * Handle an HTTP HEAD Request and generate the information necessary to assemble a Response.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param request the request
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @return 0 on success, -1 and set err on failure
 */
static int http_head(struct core_object *co, struct state_object *so, struct http_request *request, size_t *status,
                     struct http_header ***headers);

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

/**
 * store_in_fs
 * <p>
 * Store the entity body of a request in the file system at its request line.
 * </p>
 * @param co the core object
 * @param request the request
 * @return 0 on success, -1 and set err on failure
 */
static int store_in_fs(struct core_object *co, const struct http_request *request);

/**
 * post_insert_assemble_response_innards
 * <p>
 * Assemble the innards of the Response to a POST Request if an entity was inserted into the database with no
 * overwriting.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @param entity_body pointer to the entity body for the response
 * @return 0 on success, -1 on failure
 */
static int post_insert_assemble_response_innards(struct core_object *co, struct http_request *request,
                                                 size_t *status, struct http_header ***headers, char **entity_body);

/**
 * post_assemble_response_innards
 * <p>
 * Assemble the innards of the Response to a POST Request if an entity was inserted into the database with overwriting.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param status pointer to the status field for the response
 * @param headers pointer to the header list for the response
 * @param entity_body pointer to the entity body for the response
 * @return 0 on success, -1 on failure
 */
static int
post_assemble_response_innards(struct core_object *co, struct http_request *request, struct http_header ***headers,
                               char **entity_body);

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
    
    return 0;
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
    
    int                overwrite_status;
    struct http_header *database_header;
    struct http_header *content_length_header;
    
    // Read headers to determine if database or file system
    database_header = get_header("database", request->extension_headers, request->num_extension_headers);
    content_length_header = get_header("content-length", request->request_headers, request->num_request_headers);
    
    // Store with key as URI
    to_lower(database_header->value);
    if (strcmp(database_header->value, "true") == 0)
    {
        datum key;
        datum value;
        
        key.dptr    = request->request_line->request_URI;
        key.dsize   = strlen(request->request_line->request_URI);
        value.dptr  = *entity_body;
        value.dsize = strtol(content_length_header->value, NULL, 10);
        
        overwrite_status = db_upsert(co, DB_NAME, so->db_sem, &key, &value);
        
    } else
    {
        overwrite_status = store_in_fs(co, request);
    }
    
    switch (overwrite_status)
    {
        case 0: // insert no overwrite
        {
            *status = CREATED_201;
            if (post_assemble_response_innards(co, request, headers, entity_body) == -1)
            {
                return -1;
            }
            break;
        }
        case 1: // insert overwrite
        {
            *status = OK_200;
            if (post_assemble_response_innards(co, request, headers, entity_body) == -1)
            {
                return -1;
            }
            break;
        }
        case -1: // error
        {
            return -1;
        }
        default:;
    }
    
    return 0;
}

static int store_in_fs(struct core_object *co, const struct http_request *request)
{
    char pathname[BUFSIZ];
    int  overwrite_status;
    
    memset(pathname, 0, BUFSIZ);
    if (getcwd(pathname, BUFSIZ) == NULL)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    strlcat(pathname, WRITE_DIR, BUFSIZ);
    
    overwrite_status = write_to_dir(pathname, request->request_line->request_URI,
                                    request->entity_body, strlen(request->entity_body));
    if (overwrite_status == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
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
    
    *entity_body = request->entity_body;
    
    content_type   = mm_malloc(sizeof(struct http_header), co->mm);
    content_length = mm_malloc(sizeof(struct http_header), co->mm);
    *headers = mm_malloc((num_headers + 1) * sizeof(struct http_header *), co->mm);
    if (!(content_type && content_length && *headers))
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    memset(entity_body_size, 0, CONTENT_LENGTH_MAX_DIGITS);
    sprintf(entity_body_size, "%lu", strlen(*entity_body));
    
    content_type   = set_header(co, "content-type", "text/html");
    content_length = set_header(co, "content-length", entity_body_size);
    
    offset = 0;
    *(*headers + offset++) = content_type;
    *(*headers + offset++) = content_length;
    *(*headers + offset)   = NULL;
    
    return 0;
}
