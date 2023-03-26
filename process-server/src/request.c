#include <request.h>

#include <manager.h>
#include <util.h>

#include <ctype.h>

enum header_tokens{H_KEY = 0, H_VALUE = 1, H_TOKS = 2};
enum request_line_tokens{RL_METHOD = 0, RL_REQUEST_URI = 1, RL_HTTP_VERSION = 2, RL_TOKS = 3};

struct http_request * init_http_request(struct core_object * co) {
    struct http_request * req;

    req = mm_malloc(sizeof (struct http_request), co->mm);
    if (!req) {
        SET_ERROR(co->err);
        return NULL;
    }

    return req;
}

void destroy_http_request(struct http_request ** req, struct core_object * co) {
    if (*req) {
        // destroy request line
        destroy_http_request_line((*req)->request_line, co);

        // free general headers
        for (size_t i = 0; i < (*req)->num_general_headers; i++) {
            destroy_http_header((*req)->general_headers[i], co);
        }
        // free request headers
        for (size_t i = 0; i < (*req)->num_request_headers; i++) {
            destroy_http_header((*req)->request_headers[i], co);
        }
        // free entity headers
        for (size_t i = 0; i < (*req)->num_entity_headers; i++) {
            destroy_http_header((*req)->entity_headers[i], co);
        }
        // free extension headers
        for (size_t i = 0; i < (*req)->num_extension_headers; i++) {
            destroy_http_header((*req)->extension_headers[i], co);
        }
        // free entity body
        if ((*req)->entity_body) {
            mm_free(co->mm, (*req)->entity_body);
        }

        // free request struct
        mm_free(co->mm, *req);
    }
    *req = NULL;
}

struct http_header * init_http_header(char * header_line, struct core_object * co) {
    char sep[3] = {COLON, SP, TERM};
    char * toks[H_TOKS];
    char * tok;

    tok = litlittok(header_line, sep);
    int i = 0;
    while (tok != NULL) {
        tok = litlittok(NULL, sep);
        if (i < H_TOKS) {
            toks[i] = tok;
        }
        i++;
    }
    if (i + 1 != H_TOKS) {
        (void) fprintf(stderr, "unexpected number of tokens in header: expected 2, got %i\n", i + 1);
        return NULL;
    }

    struct http_header * header = mm_malloc(sizeof(struct http_header), co->mm);
    if (!header) {
        SET_ERROR(co->err);
        return NULL;
    }
    header->key = mm_malloc(strlen(toks[H_KEY]) + 1, co->mm);
    if (!header->key) {
        SET_ERROR(co->err);
        return NULL;
    }
    header->value = mm_malloc(strlen(toks[H_VALUE]) + 1, co->mm);
    if (!header->value) {
        SET_ERROR(co->err);
        return NULL;
    }

    to_lower(toks[H_KEY]); // field names are case-insensitive (RFC section 4.2)
    strcpy(header->key, toks[H_KEY]);
    strcpy(header->value, toks[H_VALUE]);

    return header;
}

void destroy_http_header(struct http_header * header, struct core_object * co) {
    if (header->key) {
        mm_free(co->mm, header->key);
    }
    if (header->value) {
        mm_free(co->mm, header->value);
    }
    mm_free(co->mm, header);
}

struct http_request_line * init_http_request_line(char * raw_request_line, struct core_object * co) {
    char * tok;
    char sep[2] = {SP, TERM};
    char * toks[RL_TOKS];

    struct http_request_line * request_line = mm_malloc(sizeof(struct http_request_line), co->mm);
    if (!request_line) {
        SET_ERROR(co->err);
        return NULL;
    }

    tok = litlittok(raw_request_line, sep);
    int i = 0;
    while(tok != NULL) {
        tok = litlittok(NULL, sep);
        if (i < RL_TOKS) {
            toks[i] = tok;
        }
        i++;
    }
    if (i + 1 != RL_TOKS) {
        (void) fprintf(stderr, "unexpected number of tokens in Request-Line: expected 3, got %i\n", i + 1);
        return NULL;
    }

    request_line->method = toks[RL_METHOD];
    request_line->request_URI = toks[RL_REQUEST_URI];
    request_line->http_version = toks[RL_HTTP_VERSION];
    return request_line;
}

void destroy_http_request_line(struct http_request_line * request_line, struct core_object * co) {
    if (request_line->http_version) {
        mm_free(co->mm, request_line->http_version);
    }
    if (request_line->method) {
        mm_free(co->mm,request_line->method);
    }
    if (request_line->request_URI) {
        mm_free(co->mm, request_line->request_URI);
    }
    mm_free(co->mm, request_line);
}
