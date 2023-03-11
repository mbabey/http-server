#include "../include/process_server.h"
#include "../include/server_state.h"

int initialize_server(struct core_object *co)
{
    PRINT_STACK_TRACE(co->tracer);
    
    if (setup_process_server(co, co->so) == -1)
    {
        return ERROR;
    }
    
    return RUN_SERVER;
}

int run_server(struct core_object *co)
{
    PRINT_STACK_TRACE(co->tracer);
    
    if (run_process_server(co, co->so) == -1)
    {
        return ERROR;
    }
    
    return CLOSE_SERVER;
}

int close_server(struct core_object *co)
{
    PRINT_STACK_TRACE(co->tracer);
    
    destroy_process_state(co, co->so);
    
    return EXIT;
}
