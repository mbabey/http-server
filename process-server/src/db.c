#include "../include/db.h"
#include "../include/manager.h"
#include "../include/util.h"

#include <stdlib.h>
#include <unistd.h>

/**
 * create_split_dir
 * <p>
 * For URIs that are long as, split the URI to create the directories necessary to store the file.
 * </p>
 * @param co the core object
 * @param new_dir_path the directory path
 * @param save_dir the save directory
 * @return 0 on success, -1 and set err on failure
 */
static int create_split_dir(struct core_object *co, char *new_dir_path, const char *save_dir);

int db_upsert(struct core_object *co, const char *db_name, sem_t *sem, datum *key, datum *value)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int status;
    int ret_val;
    DBM *db;
    
    if (sem_wait(sem) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    // NOLINTBEGIN(concurrency-mt-unsafe) : Protected
    db     = dbm_open(db_name, DB_FLAGS, DB_FILE_MODE);
    status = dbm_store(db, *key, *value, DBM_INSERT);
    if (db == (DBM *) 0)
    {
        SET_ERROR(co->err);
        sem_post(sem);
        return -1;
    }
    ret_val = status; // ret_val will be 1, 0, or -1
    if (status == 1)
    {
        status = dbm_store(db, *key, *value, DBM_REPLACE);
    }
    dbm_close(db);
    // NOLINTEND(concurrency-mt-unsafe) : Protected
    sem_post(sem);
    
    if (status == -1) // If an error occurred in the insert or replace.
    {
        print_db_error(db);
        ret_val = -1;
    }
    
    return ret_val;
}

int safe_dbm_fetch(struct core_object *co, const char *db_name, sem_t *sem, datum *key, uint8_t **serial_buffer)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int   ret_val;
    DBM   *db;
    datum value;
    
    if (sem_wait(sem) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    // NOLINTBEGIN(concurrency-mt-unsafe) : Protected
    db = dbm_open(db_name, DB_FLAGS, DB_FILE_MODE);
    if (db == (DBM *) 0)
    {
        SET_ERROR(co->err);
        sem_post(sem);
        return -1;
    }
    value = dbm_fetch(db, (*key));
    if (!value.dptr && dbm_error(db))
    {
        print_db_error(db);
    }
    ret_val = copy_dptr_to_buffer(co, serial_buffer, &value);
    dbm_close(db);
    // NOLINTEND(concurrency-mt-unsafe) : Protected
    sem_post(sem);
    
    return ret_val;
}

int copy_dptr_to_buffer(struct core_object *co, uint8_t **buffer, datum *value)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int ret_val;
    
    if (value->dptr)
    {
        *buffer = mm_malloc(value->dsize, co->mm);
        if (!*buffer)
        {
            SET_ERROR(co->err);
            return -1;
        }
        memcpy(*buffer, value->dptr, value->dsize);
        
        ret_val = 0;
    } else
    {
        ret_val = 1;
    }
    
    return ret_val;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
int write_to_dir(struct core_object *co, char *save_dir, const char *file_name, const char *data_buffer,
                 size_t data_buf_size)
{
    PRINT_STACK_TRACE(co->tracer);
    
    char *save_file_name = NULL;
    int  save_fd;
    
    if (set_string(&save_file_name, save_dir) == NULL)
    {
        SET_ERROR(co->err);
        return -1;
    }
    if (append_string(&save_file_name, file_name) == NULL)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    if (create_split_dir(co, save_file_name, save_dir) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    int ret_val;
    
    if (access(save_file_name, F_OK) == 0)
    {
        ret_val = 1;
    } else
    {
        ret_val = 0;
    }
    save_fd = open(save_file_name, O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC, WR_DIR_FLAGS);
    if (save_fd == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    if (write(save_fd, data_buffer, data_buf_size) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    close(save_fd);
    free(save_file_name);
    
    return ret_val;
}


static int create_split_dir(struct core_object *co, char *new_dir_path, const char *save_dir)
{
    PRINT_STACK_TRACE(co->tracer);
    
    char *new_dir_path_temp;
    
    new_dir_path_temp = strdup(new_dir_path);
    if (!new_dir_path_temp)
    {
        SET_ERROR(co->err);
        return -1;
    }
    
    for (size_t length = strlen(new_dir_path_temp); strcmp(new_dir_path_temp, save_dir) != 0 && length > 0; --length)
    {
        if (*(new_dir_path_temp + length) == '/')
        {
            *(new_dir_path_temp + length + 1) = '\0';
            create_dir(new_dir_path_temp);
            break;
        }
    }
    
    free(new_dir_path_temp);
    
    return 0;
}

void print_db_error(DBM *db)
{
    int err_code;
    
    err_code = dbm_error(db); // NOLINT(concurrency-mt-unsafe) : No threads here
    dbm_clearerr(db);         // NOLINT(concurrency-mt-unsafe) : No threads here
    
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers) : Numbers fine here.
    switch (err_code)
    {
        case 1:
        {
            (void) fprintf(stdout, "Database error occurred: The specified key was not found in the database.\n");
            break;
        }
        case 2:
        {
            (void) fprintf(stdout, "Database error occurred: The database file could not be opened.\n");
            break;
        }
        case 3:
        {
            (void) fprintf(stdout, "Database error occurred: The database file could not be created.\n");
            break;
        }
        case 4:
        {
            (void) fprintf(stdout,
                           "Database error occurred: An I/O error occurred while reading or writing the database file.\n");
            break;
        }
        case 5:
        {
            (void) fprintf(stdout, "Database error occurred: The database was not opened in read-write mode.\n");
            break;
        }
        case 6:
        {
            (void) fprintf(stdout, "Database error occurred: The database is already open and cannot be reopened.\n");
            break;
        }
        case 7:
        {
            (void) fprintf(stdout,
                           "Database error occurred: The specified key or value was too long to be stored in the database.\n");
            break;
        }
        case 8:
        {
            (void) fprintf(stdout, "Database error occurred: A memory allocation error occurred.\n");
            break;
        }
        case 9:
        {
            (void) fprintf(stdout, "Database error occurred: The database file format is invalid.\n");
            break;
        }
        case 10:
        {
            (void) fprintf(stdout, "Database error occurred: The database file is too old and needs to be rebuilt.\n");
            break;
        }
        case 11:
        {
            (void) fprintf(stdout, "Database error occurred: An unknown error occurred.\n");
            break;
        }
        default:;
    }
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
}

