#include <read.h>

#include <request.h>
#include <util.h>

#include <stdlib.h>
#include <mem_manager/manager.h>

enum states{SUCCESS = 0, FAILURE = -1};

/**
 * read_request_line
 * <p>
 * Reads the Request-Line section of an HTTP 1.0 request from a client connection.
 * </p>
 * @param fd the client connection.
 * @param co the core object.
 * @return 0 on success, -1 on failure.
 */
static int read_request_line(int fd, struct core_object * co);

/**
 * read_headers
 * <p>
 * Reads HTTP 1.0 headers from a client connection into a request until a CRLF is encountered.
 * </p>
 * @param the client connection.
 * @param so the core object.
 * @return 0 on success, -1 on failure.
 */
static int read_headers(int fd, struct core_object * co);

/**
 * marshal_headers
 * <p>
 * Creates and marshals headers into the request struct.
 * </p>
 * @param raw_headers string containing all headers.
 * @param co the core object.
 * @return 0 on success, -1 on failure.
 */
static int marshal_headers(char * raw_headers, struct core_object * co);

/**
 * read_entity_body
 * <p>
 * Reads the Entity-Body section of an HTTP 1.0 request from a client connection if present.
 * </p>
 * @param fd the client connection.
 * @param so the core object.
 * @return 0 on success, -1 on failure.
 */
static int read_entity_body(int fd, struct core_object * co);

/**
 * read_until
 * <p>
 * Reads a character at a time from a file descriptor until a certain character sequence occurs.
 * </p>
 * @param fd the file descriptor to read from.
 * @param until the terminating character sequence.
 * @return the read line on success, NULL on failure.
 */
static char * read_until(int fd, char * until, struct core_object * co);

int read_request(int fd, struct core_object * co) {
    if (read_request_line(fd, co) == FAILURE) {
        return -1;
    }

    if (read_headers(fd, co) == FAILURE) {
        return -1;
    }

    if (read_entity_body(fd, co) == FAILURE) {
        return -1;
    }

    return 0;
}

static int read_request_line(int fd, struct core_object * co) {
    size_t c_size = sizeof(char);
    char last_c = ' ';
    char current_c = ' ';
    char * raw_request_line;
    size_t request_line_size = c_size;

    raw_request_line = mm_malloc(c_size, co->mm, __FILE__, __FUNCTION__, __LINE__);
    if (raw_request_line == NULL) {
        SET_ERROR(co->err);
        return FAILURE;
    }

    // TODO: call read_until() here with until = CRLF

    co->so->req->request_line = init_http_request_line(raw_request_line, co);
    if (!co->so->req->request_line) {
        return FAILURE;
    }
    mm_free(raw_request_line, co);

    return SUCCESS;
}

static int read_headers(int fd, struct core_object * co) {
    char last_c = ' ';
    char current_c = ' ';
    size_t c_size = sizeof(char);
    size_t headers_size = c_size;

    char * raw_headers = mm_malloc(sizeof(char), co->mm, __FILE__, __FUNCTION__, __LINE__);
    if (!raw_headers) {
        SET_ERROR(co->err);
        return FAILURE;
    }

    // TODO: add read_until call here with until = CRLFCRLF

    return SUCCESS;
}

static int read_entity_body(int fd, struct core_object * co) {

    return SUCCESS;
}

static char * read_until(int fd, char * until, struct core_object * co) {
    size_t line_size = 2; // initial line size
    size_t line_idx = 0; // where to write the first character

    char * line = mm_malloc(line_size, co->mm, __FILE__, __FUNCTION__, __LINE__);
    if (!line) {
        SET_ERROR(co->err);
        return NULL;
    }

    do {
        // TODO: read character, realloc, write character, increment
    } while (!strstr(line, until));

    // TODO: remove until from end of line
    // TODO: add null terminator

    return line;
}
