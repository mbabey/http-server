#ifndef HTTP_SERVER_REQUEST_H
#define HTTP_SERVER_REQUEST_H

#include <objects.h>

/**
 * Represents a HTTP 1.0 header
 */
struct http_header {
    int version;
};

/**
 * Represents a HTTP 1.0 request
 */
struct http_request {
    struct http_header header;
    char * body;
};

/**
 * init_http_request
 * <p>
 * Reads a HTTP 1.0 request from a client socket into a http_request struct.
 * </p>
 * @param so the state object.
 * @return a pointer to a http_request struct on success. NULL on failure.
 */
struct http_request * init_http_request(struct state_object * so);

/**
 * destroy_http_request
 * <p>
 * Frees any dynamically allocated memory from a http_request, frees the http_request struct, and sets the pointer to NULL.
 * </p>
 * @param req a double pointer to a http_request struct.
 */
void destroy_http_request(struct http_request ** req);

/**
 * init_http_header
 * <p>
 * Reads a HTTP 1.0 header from a client socket into a http_header struct.
 * </p>
 * @param so the state object
 * @return a pointer to a http_header struct on success. NULL on failure.
 */
struct http_header * init_http_header(struct state_object * so);

/**
 * destroy_http_header
 * <p>
 * Frees any dynamically allocated memory from a http_header, frees the http_header struct, and sets the pointer to NULL.
 * </p>
 * @param header a double pointer to a http_header struct.
 */
void destroy_http_header(struct http_header ** header);

#endif //HTTP_SERVER_REQUEST_H
