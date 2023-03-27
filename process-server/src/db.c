#include "../include/db.h"
#include "../include/util.h"

#include <stdlib.h>
#include <unistd.h>

int db_upsert(struct core_object *co, struct state_object *so,
        void *key, size_t key_size, void *value, size_t value_size)
{
    PRINT_STACK_TRACE(co->tracer);
    
    int status;
    int ret_val;
    
    datum d_key;
    datum d_value;
    
    d_key.dptr = key;
    d_key.dsize = key_size;
    d_value.dptr = value;
    d_value.dsize = value_size;
    
    if (sem_wait(so->db_sem) == -1)
    {
        SET_ERROR(co->err);
        return -1;
    }
    status = dbm_store(so->db, d_key, d_value, DBM_INSERT);
    ret_val = status; // ret_val will be 1, 0, or -1
    if (status == 1)
    {
        status = dbm_store(so->db, d_key, d_value, DBM_REPLACE);
    }
    sem_post(so->db_sem);
    
    if (status == -1) // If an error occurred in the insert or replace.
    {
        print_db_error(so->db);
        ret_val = -1;
    }
    
    return ret_val;
}

int write_to_dir(char *save_dir, const char *file_name, const char *data_buffer,
                 size_t data_buf_size) // NOLINT(bugprone-easily-swappable-parameters)
{
    char *save_file_name = NULL;
    int  save_fd;
    
    if (set_string(&save_file_name, save_dir) == NULL)
    {
        return -1;
    }
    if (append_string(&save_file_name, "/") == NULL)
    {
        return -1;
    }
    if (append_string(&save_file_name, file_name) == NULL)
    {
        return -1;
    }
    
    save_fd = open(save_file_name, O_CREAT | O_WRONLY | O_CLOEXEC, WR_DIR_FLAGS);
    if (save_fd == -1)
    {
        return -1;
    }
    
    if (write(save_fd, data_buffer, data_buf_size) == -1)
    {
        return -1;
    }
    
    free(save_file_name);
    
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

