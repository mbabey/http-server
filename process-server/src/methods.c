#include "methods.h"

static int http_post(struct core_object *co, struct state_object *so, struct http_request *request,
                     size_t *status, struct http_header ***headers, char **entity_body);

static int http_head(struct core_object *co, struct state_object *so, struct http_request *request, size_t *status,
                     struct http_header ***headers);

static int http_get(struct core_object *co, struct state_object *so, struct http_request *request,
                    size_t *status, struct http_header ***headers, char **entity_body);

int perform_method(struct core_object *co, struct state_object *so, struct http_request *request,
                   size_t *status, struct http_header ***headers, char **entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    // tree on request line
    char *method;
    
    method = request->request_line->method;
    
    if (strcmp(method, "GET") == 0)
    {
        *entity_body = NULL;
        http_get(co, so, request, status, headers, entity_body);
    } else if (strcmp(method, "HEAD") == 0)
    {
        http_head(co, so, request, status, headers);
    } else if (strcmp(method, "POST") == 0)
    {
        http_post(co, so, request, status, headers, entity_body);
    } else
    {
        *status = NOT_IMPLEMENTED_501;
        *headers = NULL;
        *entity_body = NULL;
    }
    
    // fill out status, headers, and entity body
    
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
    
    return 0;
}
