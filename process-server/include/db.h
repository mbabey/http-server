#ifndef HTTP_SERVER_DB_H
#define HTTP_SERVER_DB_H

#include "objects.h"

/**
 * db_upsert
 * <p>
 * Upsert into the database.
 * </p>
 * @param co the core object
 * @param db_name the name of the database into which to upsert
 * @param sem the semaphore for the database
 * @param key the key to upsert
 * @param value the value to upsert
 * @return 0 on success and no overwrite, 1 on success and overwrite, -1 and set err on failure
 */
int db_upsert(struct core_object *co, const char *db_name, sem_t *sem, datum *key, datum *value);

/**
 * safe_dbm_fetch
 * <p>
 * Safely fetch an item from a database.
 * </p>
 * @param co the core object
 * @param db_name the name of the db from which to fetch
 * @param sem the db semaphore
 * @param key the key of the item to fetch
 * @param serial_buffer the buffer into which to copy the fetched item
 * @return 0 if successful and copy occurs, 1 if item not found, -1 and set err on failure
 */
int safe_dbm_fetch(struct core_object *co, const char *db_name, sem_t *sem, datum *key, uint8_t **serial_buffer);

/**
 * copy_dptr_to_buffer
 * <p>
 * Allocate memory for and copy the contents of a datum dptr into a buffer. If the dptr is NULL, return 1.
 * </p>
 * @param co the core object
 * @param buffer the buffer into which to allocate
 * @param value the datum from which to copy
 * @return 0 if successful and copy occurs, 1 if datum dptr is NULL, -1 and set err on failure.
 */
int copy_dptr_to_buffer(struct core_object *co, uint8_t **buffer, datum *value);

/**
 * write_to_dir
 * <p>
 * Save the file information in data_buffer to the directory path specified by save_dir under
 * the name file_name.
 * </p>
 * @param save_dir the directory to which the file will be saved
 * @param file_name the name of the file
 * @param data_buffer the file information
 * @param data_buf_size the size of the file
 * @return 0 on success, -1 on failure.
 */
int write_to_dir(struct core_object *co, char *save_dir, const char *file_name, const char *data_buffer,
                 size_t data_buf_size);

/**
 * print_db_error
 * <p>
 * Print an error message based on the error code of passed.
 * </p>
 * @param db the db from which to print an error
 */
void print_db_error(DBM *db);

#endif //HTTP_SERVER_DB_H
