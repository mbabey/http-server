#include "../include/response.h"

/**
 * assemble_status_line
 * <p>
 * Assemble the status line in the parameter http_response struct
 * </p>
 * @param co the core object
 * @param response the response object
 * @param status the status code
 * @return 0 on success, -1 and set err on failure
 */
static int assemble_status_line(struct core_object *co, struct http_response *response, size_t status);

int assemble_send_response(struct core_object *co, struct state_object *so, int socket_fd,
                           size_t status, const char **headers, const char *entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    struct http_response response;
    
    // Assemble the status line
    
    // Assemble the headers
    
    // Assemble the body
    
    // Serialize the response
    
    // Send the (whole) response
    
    return 0;
}

static int assemble_status_line(struct core_object *co, struct http_response *response, size_t status)
{

}
