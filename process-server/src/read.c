#include <manager.h>
#include <read.h>
#include <request.h>
#include <util.h>

/**
 * Read states
 */
enum states{SUCCESS = 0, FAILURE = -1};

/**
 * HTTP 1.0 general headers
 */
const char * const http_general_headers[2] = {H_DATE, H_PRAGMA};

/**
 * HTTP 1.0 request headers
 */
const char * const http_request_headers[5] = {H_AUTHORIZATION, H_FROM, H_IF_MODIFIED_SINCE, H_REFERER, H_USER_AGENT};

/**
 * HTTP 1.0 entity headers
 */
const char * const http_entity_headers[6] = {H_ALLOW, H_CONTENT_ENCODING, H_CONTENT_LENGTH, H_CONTENT_TYPE, H_EXPIRES, H_LAST_MODIFIED};

/**
 * read_headers
 * <p>
 * Reads HTTP 1.0 headers from a client connection into a request until a CRLF is encountered.
 * </p>
 * @param the client connection.
 * @param so the core object.
 * @return 0 on success, -1 on failure.
 */
static int read_headers(int fd, struct http_request * req, struct core_object * co);

/**
 * marshal_header
 * <p>
 * Creates and inserts a header into the request struct.
 * </p>
 * @param raw_header string containing a header. Example: "Content-Type: application/json".
 * @param co the core object.
 * @return 0 on success, -1 on failure.
 */
static int marshal_header(char * raw_header, struct http_request * req, struct core_object * co);

/**
 * add_header
 * <p>
 * Adds a header to a header array by reallocating the array and incrementing the num index.
 * </p>
 * @param header the header to add.
 * @param headers a pointer to the array to reallocate and add to.
 * @param num the index number associated with the header array.
 * @param co the core object.
 * @return 0 on success, -1 on failure.
 */
static int add_header(struct http_header * header, struct http_header *** headers, size_t * num, struct core_object * co);

/**
 * read_entity_body
 * <p>
 * Reads the Entity-Body section of an HTTP 1.0 request from a client connection if present.
 * </p>
 * @param fd the client connection.
 * @param so the core object.
 * @return 0 on success, -1 on failure.
 */
static int read_entity_body(int fd, struct http_request * req, struct core_object * co);

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

int read_request(int fd, struct http_request * req, struct core_object * co) {
    if (read_headers(fd, req, co) == FAILURE) {
        return -1;
    }

    if (read_entity_body(fd, req, co) == FAILURE) {
        return -1;
    }

    return 0;
}

static int read_headers(int fd, struct http_request * req, struct core_object * co) {
    bool request_line = true;
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers): size static
    char until[5] = {CR, LF, CR, LF, TERM};
    char sep[3] = {CR, LF, TERM};
    char * tok;

    char * headers = read_until(fd, until, co);
    if (!headers) {
        return FAILURE;
    }

    tok = litlittok(headers, sep);
    while(tok != NULL) {
        if (request_line) {
            req->request_line = init_http_request_line(tok, co);
            if (!req->request_line) {
                return FAILURE;
            }
            request_line = false;
        } else {
            if (marshal_header(tok, req, co) == -1) {
                return FAILURE;
            }
        }
        tok = litlittok(NULL, sep);
    }

    return SUCCESS;
}

static int marshal_header(char * raw_header, struct http_request * req, struct core_object * co) {
    struct http_header * header = init_http_header(raw_header, co);
    if (!header) {
        return FAILURE;
    }

    // general headers
    for (size_t i = 0; i < (sizeof(http_general_headers) / sizeof(http_general_headers[0])); i++) {
        if (strcmp(header->key, http_general_headers[i]) == 0) {
            if (add_header(header, &req->general_headers, &req->num_general_headers, co) == -1) {
                return FAILURE;
            }
            return SUCCESS;
        }
    }

    // request headers
    for (size_t i = 0; i < (sizeof(http_request_headers) / sizeof(http_request_headers[0])); i++) {
        if (strcmp(header->key, http_request_headers[i]) == 0) {
            if (add_header(header, &req->request_headers, &req->num_request_headers, co) == -1) {
                return FAILURE;
            }
            return SUCCESS;
        }
    }

    // entity headers
    for (size_t i = 0; i < (sizeof(http_entity_headers) / sizeof(http_entity_headers[0])); i++) {
        if (strcmp(header->key, http_entity_headers[i]) == 0) {
            if (add_header(header, &req->entity_headers, &req->num_entity_headers, co) == -1) {
                return FAILURE;
            }
            return SUCCESS;
        }
    }

    // any unrecognized header is an extension header
    if (add_header(header, &req->extension_headers, &req->num_extension_headers, co) == -1) {
        return FAILURE;
    }
    return SUCCESS;
}

static int add_header(struct http_header * header, struct http_header *** headers, size_t * num, struct core_object * co) {
    (*num)++;
    if (*num == 1) {
        *headers = mm_malloc(sizeof(struct http_header *), co->mm);
    } else {
        *headers = mm_realloc(*headers, (*num) * sizeof(struct http_header *), co->mm);
    }
    if (*headers == NULL) {
        (*num)--;
        SET_ERROR(co->err);
        return FAILURE;
    }
    (*headers)[(*num) - 1] = header;
    return SUCCESS;
}

static int read_entity_body(int fd, struct http_request * req, struct core_object * co) {
    struct http_header * c_length = get_header(H_CONTENT_LENGTH, req->entity_headers, req->num_entity_headers);
    if (c_length) {
        size_t length = strtosize_t(c_length->value);
        if (length == 0 && (errno == EINVAL || errno == ERANGE)) {
            SET_ERROR(co->err);
            return FAILURE;
        }
        req->entity_body = mm_malloc(length + 1, co->mm);
        if (!req->entity_body) {
            SET_ERROR(co->err);
            return FAILURE;
        }

        if (read_fully(fd,req->entity_body, length) == -1) {
            SET_ERROR(co->err);
            return FAILURE;
        }
        *(req->entity_body + length) = '\0';
    }
    return SUCCESS;
}

static char * read_until(int fd, char * until, struct core_object * co) {
    char c;
    size_t line_size = 2; // initial line size

    char * line = mm_malloc(line_size, co->mm);
    if (!line) {
        SET_ERROR(co->err);
        return NULL;
    }

    do {
        if (read_fully(fd, &c, sizeof(char)) == -1) {
            return NULL;
        }
        line = mm_realloc(line, line_size, co->mm);
        if (!line) {
            SET_ERROR(co->err);
            return NULL;
        }
        line[line_size - 2] = c;
        line[line_size - 1] = TERM;
        line_size++;
    } while (!strstr(line, until));

    // reallocate to remove until from line, add null terminator
    line_size -= strlen(until) + 1;
    line = mm_realloc(line, line_size, co->mm);
    if (!line) {
        SET_ERROR(co->err);
        return NULL;
    }
    line[line_size - 1] = TERM;

    return line;
}
