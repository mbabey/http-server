#include "../include/response.h"
#include "../include/manager.h"

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

/** Number of bytes taken by a status_line struct. */
#define STATUS_LINE_SIZE(status_line) \
    (strlen((status_line).version) + SP_SIZE + strlen((status_line).status_code) + SP_SIZE + strlen((status_line).reason_phrase))

/**
 * assemble_status_line
 * <p>
 * Assemble the status line in the parameter http_response struct
 * </p>
 * @param co the core object
 * @param response the response object
 * @param status the status code
 */
static void assemble_status_line(struct core_object *co, struct http_response *response, size_t status);

/**
 * serialize_http_response
 * <p>
 * Serialize an HTTP Response object into a byte sequence.
 * </p>
 * @param co the core object
 * @param dst_buffer the destination byte buffer
 * @param src_response the response to be serialized
 * @param entity_body_size the size of the entity body in the response
 * @return 0 on success, -1 and set error on failure.
 */
static int serialize_http_response(struct core_object *co, uint8_t **dst_buffer, struct http_response *src_response);

/**
 * get_header_size_bytes
 * <p>
 * Get the number of bytes in the headers.
 * </p>
 * @param headers the list of headers
 * @return the number of bytes in the headers
 */
static size_t get_header_size_bytes(struct http_header **headers, TRACER_FUNCTION_AS(tracer));

int assemble_send_response(struct core_object *co, struct state_object *so, int socket_fd, size_t status,
                           struct http_header **headers, const char *entity_body, size_t entity_body_size)
{
    PRINT_STACK_TRACE(co->tracer);
    
    struct http_response response;
    uint8_t              *serial_response;
    
    // Assemble the status line, headers, and body of the response
    assemble_status_line(co, &response, status);
    response.headers     = headers;
    response.entity_body = entity_body;
    
    // Serialize the response
    if (serialize_http_response(co, &serial_response, &response) == -1)
    {
        return -1;
    }
    
    // Send the (whole) response
    
    return 0;
}

static void assemble_status_line(struct core_object *co, struct http_response *response, size_t status)
{
    PRINT_STACK_TRACE(co->tracer);
    
    response->status_line.version = HTTP_VERSION;
    
    switch (status)
    {
        case OK_200:
        {
            response->status_line.status_code   = STATUS_CODE_OK;
            response->status_line.reason_phrase = REASON_PHRASE_OK;
            break;
        }
        case CREATED_201:
        {
            response->status_line.status_code   = STATUS_CODE_CREATED;
            response->status_line.reason_phrase = REASON_PHRASE_CREATED;
            break;
        }
        case ACCEPTED_202:
        {
            response->status_line.status_code   = STATUS_CODE_ACCEPTED;
            response->status_line.reason_phrase = REASON_PHRASE_ACCEPTED;
            break;
        }
        case NO_CONTENT_204:
        {
            response->status_line.status_code   = STATUS_CODE_NO_CONTENT;
            response->status_line.reason_phrase = REASON_PHRASE_NO_CONTENT;
            break;
        }
        case MOVED_PERMANENTLY_301:
        {
            response->status_line.status_code   = STATUS_CODE_MOVED_PERMANENTLY;
            response->status_line.reason_phrase = REASON_PHRASE_MOVED_PERMANENTLY;
            break;
        }
        case MOVED_TEMPORARILY_302:
        {
            response->status_line.status_code   = STATUS_CODE_MOVED_TEMPORARILY;
            response->status_line.reason_phrase = REASON_PHRASE_MOVED_TEMPORARILY;
            break;
        }
        case NOT_MODIFIED_304:
        {
            response->status_line.status_code   = STATUS_CODE_NOT_MODIFIED;
            response->status_line.reason_phrase = REASON_PHRASE_NOT_MODIFIED;
            break;
        }
        case BAD_REQUEST_400:
        {
            response->status_line.status_code   = STATUS_CODE_BAD_REQUEST;
            response->status_line.reason_phrase = REASON_PHRASE_BAD_REQUEST;
            break;
        }
        case UNAUTHORIZED_401:
        {
            response->status_line.status_code   = STATUS_CODE_UNAUTHORIZED;
            response->status_line.reason_phrase = REASON_PHRASE_UNAUTHORIZED;
            break;
        }
        case FORBIDDEN_403:
        {
            response->status_line.status_code   = STATUS_CODE_FORBIDDEN;
            response->status_line.reason_phrase = REASON_PHRASE_FORBIDDEN;
            break;
        }
        case NOT_FOUND_404:
        {
            response->status_line.status_code   = STATUS_CODE_NOT_FOUND;
            response->status_line.reason_phrase = REASON_PHRASE_NOT_FOUND;
            break;
        }
            // 500 is default, located at bottom of switch tree.
        case NOT_IMPLEMENTED_501:
        {
            response->status_line.status_code   = STATUS_CODE_NOT_IMPLEMENTED;
            response->status_line.reason_phrase = REASON_PHRASE_NOT_IMPLEMENTED;
            break;
        }
        case BAD_GATEWAY_502:
        {
            response->status_line.status_code   = STATUS_CODE_BAD_GATEWAY;
            response->status_line.reason_phrase = REASON_PHRASE_BAD_GATEWAY;
            break;
        }
        case SERVICE_UNAVAILABLE_503:
        {
            response->status_line.status_code   = STATUS_CODE_SERVICE_UNAVAILABLE;
            response->status_line.reason_phrase = REASON_PHRASE_SERVICE_UNAVAILABLE;
            break;
        }
        case INTERNAL_SERVER_ERROR_500:
        default:
        {
            response->status_line.status_code   = STATUS_CODE_INTERNAL_SERVER_ERROR;
            response->status_line.reason_phrase = REASON_PHRASE_INTERNAL_SERVER_ERROR;
            break;
        }
    }
}


static int serialize_http_response(struct core_object *co, uint8_t **dst_buffer, struct http_response *src_response)
{
    PRINT_STACK_TRACE(co->tracer);
    
    size_t headers_size_bytes;
    
    headers_size_bytes = get_header_size_bytes(src_response->headers, co->tracer);
    
    *dst_buffer = mm_malloc(STATUS_LINE_SIZE(src_response->status_line) + CRLF_SIZE
                            + headers_size_bytes // Includes CRLF_SIZE
                            + CRLF_SIZE
                            + strlen(src_response->entity_body),
                            co->mm);
    if (!*dst_buffer)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    size_t byte_offset;
    struct http_header **headers;
    
    byte_offset = 0;
    
    // Serialize the status line.
    // strcpy to avoid copying null byte.
    strcpy((char *) (*dst_buffer + byte_offset), src_response->status_line.version);
    byte_offset += sizeof(src_response->status_line.version);
    strcpy((char *) (*dst_buffer + byte_offset), SP_STR);
    byte_offset += SP_SIZE;
    strcpy((char *) (*dst_buffer + byte_offset), src_response->status_line.status_code);
    byte_offset += sizeof(src_response->status_line.status_code);
    strcpy((char *) (*dst_buffer + byte_offset), SP_STR);
    byte_offset += SP_SIZE;
    strcpy((char *) (*dst_buffer + byte_offset), src_response->status_line.reason_phrase);
    byte_offset += sizeof(src_response->status_line.reason_phrase);
    strcpy((char *) (*dst_buffer + byte_offset), CRLF_STR);
    byte_offset += CRLF_SIZE;
    
    // Serialize the headers.
    headers = src_response->headers;
    for (; *headers; ++headers)
    {
        strcpy((char *) (*dst_buffer + byte_offset), (*headers)->key);
        byte_offset += strlen((*headers)->key);
        strcpy((char *) (*dst_buffer + byte_offset), COLON_SP_STR);
        byte_offset += COLON_SP_SIZE;
        strcpy((char *) (*dst_buffer + byte_offset), (*headers)->value);
        byte_offset += strlen((*headers)->value);
        strcpy((char *) (*dst_buffer + byte_offset), CRLF_STR);
        byte_offset += CRLF_SIZE;
    }
    
    strcpy((char *) (*dst_buffer + byte_offset), CRLF_STR);
    byte_offset += CRLF_SIZE;
    
    strcpy((char *) (*dst_buffer + byte_offset), src_response->entity_body);
    
    return 0;
}

static size_t get_header_size_bytes(struct http_header **headers, TRACER_FUNCTION_AS(tracer))
{
    PRINT_STACK_TRACE(tracer);
    
    size_t header_bytes_total;
    
    header_bytes_total = 0;
    for (; *headers; ++headers)
    {
        header_bytes_total += strlen((*headers)->key) + COLON_SP_SIZE + strlen((*headers)->value) + CRLF_SIZE;
    }
    
    return header_bytes_total;
}
