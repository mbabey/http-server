#include "../include/db.h"
#include "../include/manager.h"
#include "../include/methods.h"
#include "../include/util.h"

#include <unistd.h>

static int http_post(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body);

static int http_head(struct core_object *co, struct state_object *so, struct http_request *request, size_t *status,
                     struct http_header ***headers);

static int http_get(struct core_object *co, struct state_object *so, struct http_request *request,
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
    
    struct http_header *database_header;
    
    // Read headers to determine if database or file system
    // TODO: which one of these is it in?
    database_header = get_header("database", request->request_headers, request->num_request_headers);
    database_header = get_header("database", request->entity_headers, request->num_entity_headers);
    database_header = get_header("database", request->general_headers, request->num_general_headers);
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
