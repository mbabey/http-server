#include "../include/response.h"

int assemble_send_response(struct core_object *co, struct state_object *so, int socket_fd,
                           size_t status, const char **headers, const char *entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    // Assemble the status line
    
    // Assemble the headers
    
    // Assemble the body
    
    // Serialize the response
    
    // Send the (whole) response
    
    return 0;
}

static int assemble_status_line
