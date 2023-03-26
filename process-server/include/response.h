#ifndef HTTP_SERVER_RESPONSE_H
#define HTTP_SERVER_RESPONSE_H

#include "objects.h"

/**
 * assemble_send_response
 * <p>
 * Assemble the HTTP response and send it to the client.
 * </p>
 * @param co the core object
 * @param so the state object
 * @param socket_fd the socket on which to send the response
 * @param status the status code of the response
 * @param headers the headers of the response, or NULL if not applicable
 * @param entity_body the body of the response, or NULL if not applicable
 * @return 0 on success, -1 and set err on failure
 */
int assemble_send_response(struct core_object *co, struct state_object *so, int socket_fd, size_t status,
                           struct http_header **headers, const char *entity_body, size_t entity_body_size);

#endif //HTTP_SERVER_RESPONSE_H
