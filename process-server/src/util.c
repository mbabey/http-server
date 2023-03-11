#include "objects.h"
#include "util.h"

#include <arpa/inet.h>
#include <mem_manager/manager.h>
#include <string.h>

/**
 * assemble_listen_addr
 * <p>
 * Assemble a the server's listen addr. Zero memory and fill fields.
 * </p>
 * @param listen_addr the address to assemble
 * @param port_num the port number
 * @param ip_addr the IP address
 * @param mm the memory manager object
 * @return 0 on success, -1 and set errno on failure.
 */
static int assemble_listen_addr(struct sockaddr_in *listen_addr, in_port_t port_num, const char *ip_addr);

void trace_reporter(const char *file_name, const char *function_name, size_t line_number)
{
    (void) fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}

int setup_core_object(struct core_object *co, const in_port_t port_num, const char *ip_addr)
{
    PRINT_STACK_TRACE(co->tracer);
    memset(co, 0, sizeof(struct core_object));
    
    co->tracer = trace_reporter;
    co->mm  = init_mem_manager();
    if (!co->mm)
    {
        // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
        (void) fprintf(stderr, "Fatal: could not initialize memory manager: %s\n", strerror(errno));
        return -1;
    }
    
    if (assemble_listen_addr(&co->listen_addr, port_num, ip_addr) == -1)
    {
        // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
        (void) fprintf(stderr, "Fatal: could not assign server address: %s\n", strerror(errno));
        return -1;
    }
    
    return 0;
}

static int assemble_listen_addr(struct sockaddr_in *listen_addr, const in_port_t port_num, const char *ip_addr)
{
    int ret_val;
    
    memset(listen_addr, 0, sizeof(struct sockaddr_in));

    listen_addr->sin_port   = htons(port_num);
    listen_addr->sin_family = AF_INET;
    switch (inet_pton(AF_INET, ip_addr, &listen_addr->sin_addr.s_addr))
    {
        case 1: // Valid
        {
            ret_val = 0;
            break;
        }
        case 0: // Not a valid IP address
        {
            (void) fprintf(stderr, "%s is not a valid IP address\n", ip_addr);
            ret_val = -1;
            break;
        }
        default: // Some other error
        {
            ret_val = -1;
            break;
        }
    }
    
    return ret_val;
}

void destroy_core_object(struct core_object *co)
{
    free_mem_manager(co->mm);
}
