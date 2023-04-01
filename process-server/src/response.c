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
 * @param response the response to be serialized
 * @param entity_body_size the size of the entity body in the response
 * @return size of serial buffer on success, 0 and set error on failure.
 */
static size_t serialize_http_response(struct core_object *co, char **dst_buffer, struct http_response *response);

/**
 * get_header_size_bytes
 * <p>
 * Get the number of bytes in the headers.
 * </p>
 * @param headers the list of headers
 * @return the number of bytes in the headers
 */
static size_t get_header_size_bytes(struct http_header **headers, TRACER_FUNCTION_AS(tracer));

int assemble_send_response(struct core_object *co, int socket_fd,
                           size_t status, struct http_header **headers, const char *entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    struct http_response response;
    size_t               serial_response_size;
    char                 *serial_response;
    
    // Assemble the status line, headers, and body of the response
    assemble_status_line(co, &response, status);
    response.headers     = headers;
    response.entity_body = entity_body;
    
    // Serialize the response
    serial_response_size = serialize_http_response(co, &serial_response, &response);
    if (serial_response_size == 0)
    {
        return -1;
    }
    
    // Send the response
    if (send(socket_fd, serial_response, serial_response_size, 0) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
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

void print_response(struct core_object *co, struct http_response *response);

static size_t serialize_http_response(struct core_object *co, char **dst_buffer, struct http_response *response)
{
    PRINT_STACK_TRACE(co->tracer);
    
    size_t headers_size_bytes;
    size_t serial_response_size;
    
    headers_size_bytes   = get_header_size_bytes(response->headers, co->tracer);
    serial_response_size = STATUS_LINE_SIZE(response->status_line) + CRLF_SIZE
                           + headers_size_bytes // Includes CRLF_SIZE
                           + CRLF_SIZE
                           + ((response->entity_body) ? strlen(response->entity_body) : 0);
    
    print_response(co, response);
    
    printf("%lu\n", serial_response_size);
    *dst_buffer = mm_malloc(serial_response_size, co->mm);
    if (!*dst_buffer)
    {
        SET_ERROR(co->err);
        return 0;
    }
    
    size_t             byte_offset;
    struct http_header **headers;
    
    byte_offset = 0;
    
    // Serialize the status line. strcpy to avoid copying null byte.
    // Format: HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    strlcpy((*dst_buffer + byte_offset), response->status_line.version,
            strlen(response->status_line.version));
    byte_offset += strlen(response->status_line.version);
    
    strlcpy((*dst_buffer + byte_offset), SP_STR, SP_SIZE);
    byte_offset += SP_SIZE;
    
    strlcpy((*dst_buffer + byte_offset), response->status_line.status_code,
            strlen(response->status_line.status_code));
    byte_offset += strlen(response->status_line.status_code);
    
    strlcpy((*dst_buffer + byte_offset), SP_STR, SP_SIZE);
    byte_offset += SP_SIZE;
    
    strlcpy((*dst_buffer + byte_offset), response->status_line.reason_phrase,
            strlen(response->status_line.reason_phrase));
    byte_offset += strlen(response->status_line.reason_phrase);
    
    printf("%lu\n", byte_offset);
    strlcpy((*dst_buffer + byte_offset), CRLF_STR, CRLF_SIZE);
    byte_offset += CRLF_SIZE;
    printf("%lu\n", byte_offset);
    
    // Serialize the headers.
    // Format: field-name ":" [ field-value ] CRLF
    if (response->headers)
    {
        for (headers = response->headers; *headers; ++headers)
        {
            strlcpy((*dst_buffer + byte_offset), (*headers)->key, strlen((*headers)->key));
            byte_offset += strlen((*headers)->key);
            
            printf("%lu\n", byte_offset);
            strlcpy((*dst_buffer + byte_offset), COLON_SP_STR, COLON_SP_SIZE);
            byte_offset += COLON_SP_SIZE;
            
            printf("%lu\n", byte_offset);
            strlcpy((*dst_buffer + byte_offset), (*headers)->value, strlen((*headers)->value));
            byte_offset += strlen((*headers)->value);
            
            printf("%lu\n", byte_offset);
            strlcpy((*dst_buffer + byte_offset), CRLF_STR, CRLF_SIZE);
            byte_offset += CRLF_SIZE;
            printf("%lu\n", byte_offset);
        }
    }
    printf("%lu\n", byte_offset);
    strlcpy((*dst_buffer + byte_offset), CRLF_STR, CRLF_SIZE);
    byte_offset += CRLF_SIZE;
    
    if (response->entity_body)
    {
        strlcpy((*dst_buffer + byte_offset), response->entity_body,
                strlen(response->entity_body));
    }
    
    return serial_response_size;
}

void print_response(struct core_object *co, struct http_response *response)
{
    PRINT_STACK_TRACE(co->tracer);
    
    struct http_header **headers;
    
    printf("%s %s %s\r\n", response->status_line.version, response->status_line.status_code,
           response->status_line.reason_phrase);
    if (response->headers)
    {
        for (headers = response->headers; *headers; ++headers)
        {
            printf("%s: %s\r\n", (*headers)->key, (*headers)->value);
        }
    }
    printf("\r\n%s", (response->entity_body) ? response->entity_body : "");
}

static size_t get_header_size_bytes(struct http_header **headers, TRACER_FUNCTION_AS(tracer))
{
    PRINT_STACK_TRACE(tracer);
    
    if (!headers)
    {
        return 0;
    }
    
    size_t header_bytes_total;
    
    header_bytes_total = 0;
    for (; *headers; ++headers)
    {
        header_bytes_total += strlen((*headers)->key) + COLON_SP_SIZE + strlen((*headers)->value) + CRLF_SIZE;
    }
    
    return header_bytes_total;
}
