#ifndef HTTP_SERVER_READ_H
#define HTTP_SERVER_READ_H

#include <objects.h>

/**
 * read_request
 * <p>
 * Reads an HTTP 1.0 request from a client connection.
 * </p>
 * @param so the state object.
 * @return 0 on success, -1 on failure.
 */
int read_request(struct state_object * so);

#endif //HTTP_SERVER_READ_H
