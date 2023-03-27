#include "methods.h"

static int http_post(void);
static int http_head(void);
static int http_get(void);

int perform_method(struct core_object *co, struct state_object *so, struct http_request *request,
                   size_t *status, struct http_header ***headers, char **entity_body)
{
    // tree on request line
    
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
