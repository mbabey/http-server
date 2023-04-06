#ifndef HTTP_SERVER_REQUEST_H
#define HTTP_SERVER_REQUEST_H

#include <objects.h>

/**
 * init_http_request
 * <p>
 * Allocate memory for an http_request struct in the memory manager.
 * </p>
 * @param co the core object.
 * @return a pointer to a http_request struct on success. NULL on failure.
 */
struct http_request * init_http_request(struct core_object * co);

/**
 * destroy_http_request
 * <p>
 * Frees any dynamically allocated memory from a http_request, frees the http_request struct, and sets the pointer to NULL.
 * </p>
 * @param req a double pointer to a http_request struct.
 * @param co the core object.
 */
void destroy_http_request(struct http_request ** req, struct core_object * co);

/**
 * init_http_header
 * <p>
 * Reads a HTTP 1.0 header from a client socket into a http_header struct.
 * </p>
 * @param header_line a whole header line. Example: header_line = "Content-Type: application/json"
 * @param co the core object.
 * @return a pointer to a http_header struct on success. NULL on failure.
 */
struct http_header * init_http_header(char * header_line, struct core_object * co);

/**
 * init_http_request_line
 * <p>
 * Allocates memory for an http_request_line struct and marshals data into it from the raw request line.
 * </p>
 * @param raw_request_line the raw request line.
 * @param co the core object.
 * @return a pointer to a http_request_line struct.
 */
struct http_request_line * init_http_request_line(char * raw_request_line, struct core_object * co);

/**
 * destroy_http_request_line
 * <p>
 * Frees any dynamically allocated memory from an http_request_line and frees the http_request_line struct.
 * </p>
 * @param request_line pointer to a http_request_line struct.
 * @param co the core object.
 */
void destroy_http_request_line(struct http_request_line * request_line, struct core_object * co);

#endif //HTTP_SERVER_REQUEST_H
