#include <read.h>

/**
 * read_headers
 * <p>
 * Reads HTTP 1.0 headers from a client connection into a request until a CRLF is encountered.
 * </p>
 * @param so the state object.
 * @return 0 on success, -1 on failure.
 */
static int read_headers(struct state_object * so);

int read_request(struct state_object * so) {

    return 0;
}
