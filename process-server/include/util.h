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
 * litlittok
 * <p>
 * Combination of strtok and strstr, tokenize with a multi character separator.
 * </p>
 * @param str the string to tokenize.
 * @param sep the separator to use.
 * @return the token when found, NULL when not.
 */
char * litlittok(char * str, char * sep);

/**
 * to_lower
 * <p>
 * Transforms a string to lowercase.
 * </p>
 * @param s the string to transform.
 */
void to_lower(char * s);

#endif //POLL_SERVER_UTIL_H
