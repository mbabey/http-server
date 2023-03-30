#ifndef POLL_SERVER_UTIL_H
#define POLL_SERVER_UTIL_H

#include <objects.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>

#define WR_DIR_FLAGS (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

#define HTTP_TIME_LEN 256 // TODO: too big, could be more precise

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
 * set_header
 * <p>
 * Allocate memory for and set the values in an http_header struct. Allocates memory within the co->mm.
 * </p>
 * @param co the core object
 * @param key the key of the header
 * @param value the value of the header
 * @return the header, or NULL and set err on failure.
 */
struct http_header *set_header(struct core_object *co, const char *key, const char *value);

/**
 * destroy_http_header
 * <p>
 * Frees any dynamically allocated memory from an http_header and frees the http_header struct.
 * </p>
 * @param header a pointer to a http_header struct.
 * @param co the core object.
 */
void destroy_http_header(struct http_header * header, struct core_object * co);

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

/**
 * create_dir_str
 * <p>
 * Append to the string stored as the write directory the IP address of the connected client.
 * Store this string in a new string pointer, preserving the base write directory string stored in
 * server_settings for the next client.
 * </p>
 * @param save_dir - char **: pointer to the string to hold the write directory for this client
 * @param wr_dir - char *: the base write directory stored in server_settings
 * @param client_addr_str - char*: the client's IP address
 */
char * create_dir_str(char **save_dir, const char *wr_dir, const char *client_addr_str);

/**
 * create_dir
 * <p>
 * Create the directory path specified by the string save_dir.
 * </p>
 * @param save_dir - char *: the directory path to which files will be saved for this client
 */
int create_dir(const char *save_dir);

/**
 * http_time_now
 * <p>
 * Creates an HTTP 1.0 compliant time from the current time.
 * </p>
 * @param dst where to write the time.
 * @return 0 on success, -1 on failure.
 */
int http_time_now(char dst[HTTP_TIME_LEN]);

/**
 * http_time_to_time_t
 * <p>
 * Converts an HTTP 1.0 time string to a time_t value.
 * </p>
 * @param http_time the string to convert.
 * @return the time value on success, -1 on failure.
 */
time_t http_time_to_time_t(char http_time[HTTP_TIME_LEN]);

/**
 * compare_http_time
 * <p>
 * Compares two HTTP 1.0 times.
 * </p>
 * @param time_1 the first time to compare.
 * @param time_2 the second time to compare.
 * @return 1 if time_1 > time_2, and 0 if time_2 > time_1. -1 on error.
 */
int compare_http_time(char time_1[HTTP_TIME_LEN], char time_2[HTTP_TIME_LEN]);

#endif //POLL_SERVER_UTIL_H
