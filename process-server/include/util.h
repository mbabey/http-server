#ifndef POLL_SERVER_UTIL_H
#define POLL_SERVER_UTIL_H

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>

/**
 * write_fully
 * <p>
 * writes data fully to a file descriptor.
 * </p>
 * @param fd file descriptor to write to.
 * @param data data to write.
 * @param size size of data.
 * @return 0 on success. On failure -1 and set errno.
 */
int write_fully(int fd, void * data, size_t size);

/**
 * read_fully
 * <p>
 * reads data fully from a file descriptor.
 * </p>
 * @param fd file descriptor to read from.
 * @param data where to write read data.
 * @param size size of data to read.
 * @return 0 on success. On failure -1 and set errno.
 */
int read_fully(int fd, void * data, size_t size);

/**
 * set_string
 * <p>
 * Set a string to a value. If the string is NULL, will call malloc. If the string is not NULL, will call
 * realloc.
 * </p>
 * <p>
 * NOTE: Requires a dynamically allocated or NULL first parameter.
 * </p>
 * <p>
 * <h3>
 * WARNING: set_string dynamically allocates memory. Must free the pointer passed as the first parameter!
 * </h3>
 * </p>
 * @param str - char**: pointer to the string to be set
 * @param new_str - char*: the new value for the string
 * @return the string, or NULL on failure
 */
char * set_string(char **str, const char *new_str);

/**
 * prepend_string
 * <p>
 * Prepend a prefix to a string.
 * </p>
 * <p>
 * NOTE: Requires a dynamically allocated or NULL first parameter.
 * </p>
 * @param str - char**: pointer to the string to be modified
 * @param prefix - char*: the string to be prepended
 * @return the modified string, or NULL on failure
 */
char *prepend_string(char **str, const char *prefix);

/**
 * append_string
 * <p>
 * Append a suffix to a string.
 * </p>
 * <p>
 * NOTE: Requires a dynamically allocated or NULL first parameter.
 * </p>
 * @param str - char**: pointer to the string to be modified
 * @param suffix - char*: the string to be appended
 * @return the modified string, or NULL on failure
 */
char *append_string(char **str, const char *suffix);

#endif //POLL_SERVER_UTIL_H
