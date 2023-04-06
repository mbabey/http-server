#ifndef HTTP_SERVER_METHODS_H
#define HTTP_SERVER_METHODS_H

#include "objects.h"

int perform_method(struct core_object *co, struct state_object *so, struct http_request *request,
        size_t *status, struct http_header ***headers, char **entity_body);

#endif //HTTP_SERVER_METHODS_H
