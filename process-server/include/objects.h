#ifndef PROCESS_SERVER_OBJECTS_H
#define PROCESS_SERVER_OBJECTS_H

#include "error_handlers.h"

#include <semaphore.h>
#include <poll.h>
#include <netinet/in.h>

/**
 * HTTP 1.0 methods
 */
#define M_GET "GET"
#define M_HEAD "HEAD"
#define M_POST "POST"

/**
 * HTTP 1.0 headers
 */
#define H_ALLOW "Allow"
#define H_AUTHORIZATION "Authorization"
#define H_CONTENT_ENCODING "Content-Encoding"
#define H_CONTENT_LENGTH "Content-Length"
#define H_CONTENT_TYPE "Content-Type"
#define H_DATE "Date"
#define H_EXPIRES "Expires"
#define H_FORM "Form"
#define H_IF_MODIFIED_SINCE "If-Modified-Since"
#define H_LAST_MODIFIED "Last-Modified"
#define H_LOCATION "Location"
#define H_PRAGMA "Pragma"
#define H_REFERER "Referer"
#define H_SERVER "Server"
#define H_USER_AGENT "User-Agent"
#define H_WWW_AUTHENTICATE "WWW-Authenticate"

/**
 * HTTP 1.0 syntax
 */
#define CR '\r'
#define LF '\n'
#define SP ' '
#define TERM '\0'

/**
 * The number of worker processes to be spawned to handle network requests.
 */
#define NUM_CHILD_PROCESSES 8

/**
* The number of connections that can be queued on the listening socket.
*/
#define CONNECTION_QUEUE 100

/**
* The maximum number of connections that can be accepted by the process server.
*/
#define MAX_CONNECTIONS 5

/**
 * The size of the pollfds array. +2 for listen socket and child-to-parent pipe.
 */
#define POLLFDS_SIZE 2 + MAX_CONNECTIONS

/**
* Read end of child_finished_pipe or read child_finished_semaphore.
*/
#define READ 0

/**
* Write end of child_finished_pipe or read child_finished_semaphore.
*/
#define WRITE 1

/**
* Pipe write semaphore name.
*/
#define PIPE_WRITE_SEM_NAME "/pw_2f6a08" // Random hex to prevent collision of this filename with others.

/**
* Domain socket read semaphore name.
*/
#define DOMAIN_READ_SEM_NAME "/dr_2f6a08"

/**
* Domain socket write semaphore name.
*/
#define DOMAIN_WRITE_SEM_NAME "/dw_2f6a08"

/**
 * For each loop macro for looping over child processes.
 */
#define FOR_EACH_CHILD_c_IN_CHILD_PIDS for (size_t c = 0; c < NUM_CHILD_PROCESSES; ++c)

/**
 * For each loop macro for looping over socket pollfds.
 */
#define FOR_EACH_SOCKET_POLLFD_p_IN_POLLFDS for (size_t p = 2; p < POLLFDS_SIZE; ++p)

/**
 * core_object
 * <p>
 * Holds the core information for the execution of the framework, regardless
 * of the library loaded. Includes dc_env, dc_error, memory_manager, log file,
 * and state_object. state_object contains library-dependent data, and will be
 * assigned and handled by the loaded library.
 * </p>
 */
struct core_object {
    TRACER_FUNCTION_AS(tracer);
    struct error_saver err;
    struct memory_manager *mm;
    struct sockaddr_in listen_addr;
    
    struct state_object *so;
};

/**
 * Contains information about the program state.
 */
struct state_object
{
    pid_t                child_pids[NUM_CHILD_PROCESSES];
    int                  domain_fds[2];
    int                  c_to_p_pipe_fds[2];
    sem_t                *domain_sems[2];
    sem_t                *c_to_p_pipe_sem_write;
    struct parent_struct *parent;
    struct child_struct  *child;
    struct http_request  *req;
};

/**
 * Contains information about the parent state.
 */
struct parent_struct
{
    struct pollfd      pollfds[POLLFDS_SIZE]; // 0th position is the listen socket fd, 1st position is pipe.
    struct sockaddr_in client_addrs[MAX_CONNECTIONS];
    size_t             num_connections;
};

/**
 * Contains information about the child state.
 */
struct child_struct
{
    int                client_fd_parent;
    int                client_fd_local;
    struct sockaddr_in client_addr;
};

/**
 * Represents an HTTP 1.0 header
 */
struct http_header {
    char * key;
    char * value;
};

/**
 * Represents an HTTP 1.0 request line
 */
struct http_request_line {
    char * method;
    char * request_URI;
    char * http_version;
};

/**
 * Represents an HTTP 1.0 request
 */
struct http_request {
    struct http_request_line * request_line;
    size_t num_general_headers;
    struct http_header * general_headers;
    size_t num_request_headers;
    struct http_header * request_headers;
    size_t num_entity_headers;
    struct http_header * entity_headers;
    char * entity_body;
};

#endif //PROCESS_SERVER_OBJECTS_H
