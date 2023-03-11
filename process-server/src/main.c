#include "../include/core.h"
#include "../include/server_state.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int                next_state;
    int                run;
    struct core_object co;
    
    next_state = setup_core_object(&co, argc, argv);
    if (next_state == -1)
    {
        return EXIT_FAILURE;
    }
    
    run = 1;
    while (run)
    {
        switch (next_state)
        {
            case INITIALIZE_SERVER:
            {
                next_state = initialize_server(&co);
                break;
            }
            case RUN_SERVER:
            {
                next_state = run_server(&co);
                break;
            }
            case CLOSE_SERVER:
            {
                next_state = close_server(&co);
                break;
            }
            case ERROR:
            {
                pid_t pid;
                
                pid = getpid();
                // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
                (void) fprintf(stderr, "Fatal: error during process %d runtime: ", pid);
                GET_ERROR(co.err);
                next_state = close_server(&co);
                break;
            }
            case EXIT:
            {
                run = 0;
                break;
            }
            default: // Should not get here.
            {
                run = 0;
            }
        }
    }
    
    destroy_core_object(&co);
    
    return next_state;
}
