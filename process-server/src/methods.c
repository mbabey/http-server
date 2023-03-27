#include "methods.h"

static int http_post(void);
static int http_head(void);
static int http_get(void);

int perform_method(struct core_object *co, struct state_object *so, struct http_request *request,
                   size_t *status, struct http_header ***headers, char **entity_body)
{
    // tree on request line
    char *method;
    
    method = request->request_line->method;
    
    if (strcmp(method, "GET") == 0)
    {
        http_get();
    } else if (strcmp(method, "HEAD") == 0)
    {
        http_head();
    } else if (strcmp(method, "POST") == 0)
    {
        http_post();
    } else
    {
        *status = NOT_IMPLEMENTED_501;
        *headers = NULL;
        *entity_body = NULL;
    }
    
    // fill out status, headers, and entity body
    
    return 0;
}

static int http_get(void)
{
    return 0;
}

static int http_head(void)
{
    return 0;
}

static int http_post(void)
{
    return 0;
}
