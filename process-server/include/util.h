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
 * write_to_dir
 * <p>
 * Save the file information in data_buffer to the directory path specified by save_dir under
 * the name file_name.
 * </p>
 * @param save_dir - char *: the directory to which the file will be saved
 * @param file_name - char *: the name of the file
 * @param data_buffer - char *: the file information
 * @param data_buf_size - uint32_t: the size of the file
 */
int write_to_dir(char *save_dir, const char *file_name, const char *data_buffer, size_t data_buf_size);

#endif //POLL_SERVER_UTIL_H
