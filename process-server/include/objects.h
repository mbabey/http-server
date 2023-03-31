#ifndef PROCESS_SERVER_OBJECTS_H
#define PROCESS_SERVER_OBJECTS_H

#include "error_handlers.h"

#include <semaphore.h>
#include <poll.h>
#include <netinet/in.h>
#include <ndbm.h>

#define HTTP_VERSION "HTTP/1.0" /** HTTP Version 1.0 */

/**
 * HTTP 1.0 methods
 */
#define M_GET "GET"
#define M_HEAD "HEAD"
#define M_POST "POST"

/**
 * HTTP 1.0 headers
 */
#define H_ALLOW "allow"
#define H_AUTHORIZATION "authorization"
#define H_CONTENT_ENCODING "content-encoding"
#define H_CONTENT_LENGTH "content-length"
#define H_CONTENT_LENGTH_LENGTH 12
#define H_CONTENT_TYPE "content-type"
#define H_DATE "date"
#define H_EXPIRES "expires"
#define H_FROM "from"
#define H_IF_MODIFIED_SINCE "if-modified-since"
#define H_LAST_MODIFIED "last-modified"
#define H_LOCATION "location"
#define H_PRAGMA "pragma"
#define H_REFERER "referer"
#define H_SERVER "server"
#define H_USER_AGENT "user-agent"
#define H_WWW_AUTHENTICATE "www-authenticate"

/**
 * HTTP 1.0 syntax
 */
#define CR '\r'
#define LF '\n'
#define SP ' '
#define TERM '\0'
#define COLON ':'

#define CRLF_STR "\r\n"
#define SP_STR " "
#define COLON_SP_STR ": "

#define SP_SIZE 1       /** Number of bytes for SP. */
#define CRLF_SIZE 2     /** Number of bytes for CRLF. */
#define COLON_SP_SIZE 2 /** Number of bytes for ": " */

/**
 * Misc
 */
#define TEXT_HTML_CONTENT_TYPE "text/html"

#define NUM_CHILD_PROCESSES 8             /** The number of worker processes to be spawned to handle network requests. */
#define CONNECTION_QUEUE 100              /** The number of connections that can be queued on the listening socket. */
#define MAX_CONNECTIONS 5                 /** The maximum number of connections that can be accepted by the process server. */
#define POLLFDS_SIZE 2 + MAX_CONNECTIONS  /** The size of the pollfds array. +2 for listen socket and child-to-parent pipe. */

#define READ 0   /** Read end of child_finished_pipe or read child_finished_semaphore. */
#define WRITE 1  /** Write end of child_finished_pipe or read child_finished_semaphore. */

#define PIPE_WRITE_SEM_NAME "/pw_2f6b08"      /** Pipe write semaphore name. */
#define DOMAIN_READ_SEM_NAME "/dr_2f6b08"     /** Domain socket read semaphore name. */
#define DOMAIN_WRITE_SEM_NAME "/dw_2f6b08"    /** Domain socket write semaphore name. */
#define DB_WRITE_SEM_NAME "/db_2f6b08"        /** Database socket write semaphore name. */

#define DB_NAME "db_http_2f6b08"              /** Database file name. */
#define WRITE_DIR "dir_http_2f6b08"           /** Directory name. */

#define DB_FLAGS O_RDWR | O_CREAT          /** Flags for opening db. */
#define DB_FILE_MODE S_IRUSR | S_IWUSR     /** File mode for opening db. */

#define FOR_EACH_CHILD_c_IN_CHILD_PIDS for (size_t c = 0; c < NUM_CHILD_PROCESSES; ++c) /** For each loop macro for looping over child processes. */
#define FOR_EACH_SOCKET_POLLFD_p_IN_POLLFDS for (size_t p = 2; p < POLLFDS_SIZE; ++p)   /** For each loop macro for looping over socket pollfds. */

/** HTTP 1.0 Common Status Codes. */
enum StatusCodes
{
    OK_200                    = 200,
    CREATED_201,
    ACCEPTED_202,
    NULL_203,
    NO_CONTENT_204,
    MOVED_PERMANENTLY_301     = 301,
    MOVED_TEMPORARILY_302,
    NULL_303,
    NOT_MODIFIED_304,
    BAD_REQUEST_400           = 400,
    UNAUTHORIZED_401,
    NULL_402,
    FORBIDDEN_403,
    NOT_FOUND_404,
    INTERNAL_SERVER_ERROR_500 = 500,
    NOT_IMPLEMENTED_501,
    BAD_GATEWAY_502,
    SERVICE_UNAVAILABLE_503
};

/**
 * core_object
 * <p>
 * Holds the core information for the execution of the framework, regardless
 * of the library loaded. Includes dc_env, dc_error, memory_manager, log file,
 * and state_object. state_object contains library-dependent data, and will be
 * assigned and handled by the loaded library.
 * </p>
 */
struct core_object
{
    TRACER_FUNCTION_AS(tracer);
    
    struct error_saver    err;
    struct memory_manager *mm;
    struct sockaddr_in    listen_addr;
    
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
    sem_t                *db_sem;
    DBM                  *db;
    struct parent_struct *parent;
    struct child_struct  *child;
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
struct http_header
{
    char *key;
    char *value;
};

/**
 * Represents an HTTP 1.0 request line
 */
struct http_request_line
{
    char *method;
    char *request_URI;
    char *http_version;
};

/**
 * Represents an HTTP 1.0 request
 */
struct http_request
{
    struct http_request_line *request_line;
    size_t num_general_headers;
    struct http_header **general_headers;
    size_t num_request_headers;
    struct http_header **request_headers;
    size_t num_entity_headers;
    struct http_header **entity_headers;
    size_t num_extension_headers;
    struct http_header **extension_headers;
    char               *entity_body;
};

/**
 * HTTP Response status line.
 */
struct http_status_line
{
    const char *version;
    const char *status_code;
    const char *reason_phrase;
};

/**
 * HTTP Response.
 */
struct http_response
{
    struct http_status_line status_line;
    struct http_header **headers;
    const char *entity_body;
};

#endif //PROCESS_SERVER_OBJECTS_H
