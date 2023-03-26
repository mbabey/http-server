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
 * @return
 */
int assemble_send_response(struct core_object *co, struct state_object *so, int socket_fd);

#endif //HTTP_SERVER_RESPONSE_H
