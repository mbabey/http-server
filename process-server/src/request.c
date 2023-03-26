#include <request.h>

#include <mem_manager/manager.h>

struct http_request * init_http_request(struct core_object * co) {
    struct http_request * req;

    req = mm_malloc(sizeof (struct http_request), co->mm, __FILE__, __FUNCTION__, __LINE__);
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
            destroy_http_header(&(*req)->general_headers[i], co);
        }
        // free request headers
        for (size_t i = 0; i < (*req)->num_request_headers; i++) {
            destroy_http_header(&(*req)->request_headers[i], co);
        }
        // free entity headers
        for (size_t i = 0; i < (*req)->num_entity_headers; i++) {
            destroy_http_header(&(*req)->entity_headers[i], co);
        }

        // free entity body
        mm_free((*req)->entity_body);

        // free request struct
        mm_free(*req, co->mm);
    }
    *req = NULL;
}

struct http_header * init_http_header(char * header_line, struct core_object * co) {

    return 0;
}

void destroy_http_header(struct http_header * header, struct core_object * co) {
    if (header->key) {
        mm_free(header->key, co->mm);
    }
    if (header->value) {
        mm_free(header->value, co->mm);
    }
    mm_free(header, co->mm);
}

struct http_request_line * init_http_request_line(char * raw_request_line, struct core_object * co) {
    char * tok;
    char sep[2] = {SP, TERM};
    char * toks[3];

    struct http_request_line * request_line = mm_malloc(sizeof(struct http_request_line), co->mm, __FILE__, __FUNCTION__, __LINE__);
    if (!request_line) {
        SET_ERROR(co->err);
        return NULL;
    }

    tok = strtok(raw_request_line, sep);
    int i = 0;
    while( tok != NULL ) {
        tok = strtok(NULL, sep);
        toks[i] = tok;
        i++;
    }
    if (i != 2) {
        (void) fprintf(stderr, "unexpected number of tokens in Request-Line: expected 3, got %i\n", i);
        return NULL;
    }

    request_line->method = toks[0];
    request_line->request_URI = toks[1];
    request_line->http_version = toks[2];
    return request_line;
}

void destroy_http_request_line(struct http_request_line * request_line, struct core_object * co) {
    if (request_line->http_version) {
        mm_free(request_line->http_version, co->mm);
    }
    if (request_line->method) {
        mm_free(request_line->method, co->mm);
    }
    if (request_line->request_URI) {
        mm_free(request_line->request_URI, co->mm);
    }
    mm_free(request_line, co->mm);
}
