#ifndef HTTP_SERVER_READ_H
#define HTTP_SERVER_READ_H

#include <objects.h>

/**
 * read_request
 * <p>
 * Reads an HTTP 1.0 request from a client connection.
 * </p>
 * @param fd the client connection.
 * @param co the core object.
 * @return 0 on success, -1 on failure.
 */
int read_request(int fd, struct core_object * co);

#endif //HTTP_SERVER_READ_H
