#ifndef CHAT_TEST_SADDLE_ERROR_HANDLERS_H
#define CHAT_TEST_SADDLE_ERROR_HANDLERS_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define TRACER_FUNCTION_AS(name) void (*name)(const char *, const char *, size_t)

#define PRINT_STACK_TRACE(tracer) if (tracer) tracer(__FILE__, __func__, __LINE__)

#define SET_ERROR(err) \
    err.file = __FILE__; err.func = __func__; err.line = __LINE__; err.error_number = errno

#define GET_ERROR(err) \
    (void) fprintf(stderr, "Error: %d - \"%s\" in %s:%s @ %lu\n", err.error_number, strerror(err.error_number), err.file, err.func, err.line)

struct error_saver {
    const char *file;
    const char *func;
    size_t line;
    int error_number;
};

#endif //CHAT_TEST_SADDLE_ERROR_HANDLERS_H
