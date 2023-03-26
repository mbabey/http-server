#include "../include/response.h"


/** HTTP 1.0 Status Codes and Reason Phrases */
#define STATUS_CODE_OK                      "200"
#define REASON_PHRASE_OK                    "OK"
#define STATUS_CODE_CREATED                 "201"
#define REASON_PHRASE_CREATED               "Created"
#define STATUS_CODE_ACCEPTED                "202"
#define REASON_PHRASE_ACCEPTED              "Accepted"
#define STATUS_CODE_NO_CONTENT              "204"
#define REASON_PHRASE_NO_CONTENT            "No Content"
#define STATUS_CODE_MOVED_PERMANENTLY       "301"
#define REASON_PHRASE_MOVED_PERMANENTLY     "Moved Permanently"
#define STATUS_CODE_MOVED_TEMPORARILY       "302"
#define REASON_PHRASE_MOVED_TEMPORARILY     "Moved Temporarily"
#define STATUS_CODE_NOT_MODIFIED            "304"
#define REASON_PHRASE_NOT_MODIFIED          "Not Modified"
#define STATUS_CODE_BAD_REQUEST             "400"
#define REASON_PHRASE_BAD_REQUEST           "Bad Request"
#define STATUS_CODE_UNAUTHORIZED            "401"
#define REASON_PHRASE_UNAUTHORIZED          "Unauthorized"
#define STATUS_CODE_FORBIDDEN               "403"
#define REASON_PHRASE_FORBIDDEN             "Forbidden"
#define STATUS_CODE_NOT_FOUND               "404"
#define REASON_PHRASE_NOT_FOUND             "Not Found"
#define STATUS_CODE_INTERNAL_SERVER_ERROR   "500"
#define REASON_PHRASE_INTERNAL_SERVER_ERROR "Internal Server Error"
#define STATUS_CODE_NOT_IMPLEMENTED         "501"
#define REASON_PHRASE_NOT_IMPLEMENTED       "Not Implemented"
#define STATUS_CODE_BAD_GATEWAY             "502"
#define REASON_PHRASE_BAD_GATEWAY           "Bad Gateway"
#define STATUS_CODE_SERVICE_UNAVAILABLE     "503"
#define REASON_PHRASE_SERVICE_UNAVAILABLE   "Service Unavailable"

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
