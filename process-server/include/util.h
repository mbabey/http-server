#ifndef POLL_SERVER_UTIL_H
#define POLL_SERVER_UTIL_H

#include <objects.h>
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
 * h_litlittok
 * <p>
 * Combination of strtok and strstr, tokenize with a multi character separator. HEADER VERSION.
 * </p>
 * @param str the string to tokenize.
 * @param sep the separator to use.
 * @return the token when found, NULL when not.
 */
char * h_litlittok(char * str, char * sep);

/**
 * rl_litlittok
 * <p>
 * Combination of strtok and strstr, tokenize with a multi character separator. READLINE VERSION.
 * </p>
 * @param str the string to tokenize.
 * @param sep the separator to use.
 * @return the token when found, NULL when not.
 */
char * rl_litlittok(char * str, char * sep);

/**
 * to_lower
 * <p>
 * Transforms a string to lowercase.
 * </p>
 * @param s the string to transform.
 */
void to_lower(char * s);

/**
 * get_header
 * <p>
 * Attempts to get the value of a header from a header array using its field name.
 * </p>
 * @param key the headers field name.
 * @param headers array of headers to search.
 * @param num_headers the number of headers in the header array.
 * @return the header pointer if found, NULL if not.
 */
struct http_header * get_header(const char * key, struct http_header ** headers, size_t num_headers);

/**
 * strtosize_t
 * <p>
 * Parses and returns a size_t value from a string.
 * </p>
 * @param str the string to parse.
 * @return the value if success, 0 if failure.
 */
size_t strtosize_t(char * str);

/**
 * trim_whitespace
 * <p>
 * Trims any leading or trailing whitespace from a string.
 * </p>
 * @param str the string to trim.
 * @return the trimmed string.
 */
char * trim_whitespace(char * str);

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
