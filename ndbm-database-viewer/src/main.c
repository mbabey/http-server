#include "../../process-server/include/error_handlers.h"
#include "../../process-server/include/manager.h"

#include <ctype.h>
#include <getopt.h>
#include <ndbm.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/** The command line flags. */
#define OPTS_LIST "d:s:t"

/** Usage message to print when bad user input. */
#define USAGE_MESSAGE \
    "usage: ./ndbm-database-viewer -d <database-name> [-s <semaphore-name>] [-t]\n" \
    "\t-d <database-name>: The path to the database to view.\n"\
    "\t[-s <semaphore-name>]: The database semaphore name.\n"\
    "\t[-t]: Trace the program execution.\n\n"

#define DB_FLAGS (O_RDWR | O_CREAT)          /** Flags for opening db. */
#define DB_FILE_MODE (S_IRUSR | S_IWUSR)     /** File mode for opening db. */

/** The format in which to print database entries. */
#define PRINT_FORMAT "{ key: \"%s\", value: \"%s %s\" }\n"

/** Function call to print a database entry. */
#define PRINT_ENTRY(key, value) (void) fprintf(stdout, PRINT_FORMAT, (char *) (key).dptr, (char *) (value).dptr, ((char *) (value).dptr + strlen((char *) (value).dptr) + 1))

/**
 * The program state. Holds necessary global program information.
 */
struct program_state
{
    struct error_saver    err;
    struct memory_manager *mm;
    
    TRACER_FUNCTION_AS(tracer);
    
    sem_t *db_sem;
    char  *db_sem_name;
    char  *db_name;
};

/**
 * parse_args
 * <p>
 * Parse command line arguments and set up the program state.
 * </p>
 * @param argc the number of arguments
 * @param argv the arguments
 * @param ps the program state
 * @return 0 on success, -1 and set err on failure
 */
static int parse_args(int argc, char **argv, struct program_state *ps);

/**
 * trace_reporter
 * <p>
 * Print out the file, function, and line.
 * </p>
 * @param file the file
 * @param func the function
 * @param line the line
 */
static void trace_reporter(const char *file, const char *func, size_t line);

/**
 * setup_program_state_variables
 * <p>
 * Setup the program state.
 * </p>
 * @param ps the program state
 * @param database_name_str the database name
 * @param semaphore_name_str the semaphore name
 * @return 0 on success, -1 and set err on failure
 */
static int setup_program_state_variables(struct program_state *ps, const char *database_name_str,
                                         const char *semaphore_name_str);

/**
 * destroy_program_state
 * <p>
 * Free memory allocated for the server.
 * </p>
 * @param ps the program state
 */
static void destroy_program_state(struct program_state *ps);

/**
 * scan_database
 * <p>
 * Scan the databse and print the entries.
 * </p>
 * @param ps the program state
 * @return 0 on success, -1 and set err on failure
 */
static int scan_database(struct program_state *ps);

/**
 * print_db_error
 * <p>
 * Print an error message based on the error code of passed.
 * </p>
 * @param err_code the error code
 */
static void print_db_error(DBM *db);

int main(int argc, char **argv)
{
    int                  status;
    struct program_state ps;
    
    memset(&ps, 0, sizeof(ps));
    ps.mm = init_mem_manager();
    status = parse_args(argc, argv, &ps);
    
    if (status == 0)
    {
        status = scan_database(&ps);
    }
    
    if (status == -1)
    {
        if (ps.err.error_number)
        {
            GET_ERROR(ps.err);
        }
        status = EXIT_FAILURE;
    } else
    {
        status = EXIT_SUCCESS;
    }
    
    destroy_program_state(&ps);
    free_mem_manager(ps.mm);
    
    return status;
}

static int parse_args(int argc, char **argv, struct program_state *ps)
{
    if (argc < 1)
    {
        (void) fprintf(stdout, USAGE_MESSAGE);
        return -1;
    }
    
    int        c;
    const char *database_name_str;
    const char *semaphore_name_str;
    
    database_name_str  = NULL;
    semaphore_name_str = NULL;
    
    while ((c = getopt(argc, argv, OPTS_LIST)) != -1) // NOLINT(concurrency-mt-unsafe) : No threads here
    {
        switch (c)
        {
            case 'd':
            {
                database_name_str = optarg;
                break;
            }
            case 's':
            {
                semaphore_name_str = optarg;
                break;
            }
            case 't':
            {
                ps->tracer = trace_reporter;
                break;
            }
            case '?':
            {
                if (isprint(optopt))
                {
                    // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
                    (void) fprintf(stderr, "Unknown option \'-%c\'.\n", optopt);
                } else
                {
                    // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
                    (void) fprintf(stderr, "Unknown option character \'\\x%x\'.\n", optopt);
                }
                // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
                (void) fprintf(stdout, USAGE_MESSAGE);
                break;
            }
            default:;
        }
    }
    
    if (setup_program_state_variables(ps, database_name_str, semaphore_name_str) == -1)
    {
        return -1;
    }
    
    return 0;
}

static void trace_reporter(const char *file, const char *func, size_t line)
{
    (void) fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file, func, line);
}

static int setup_program_state_variables(struct program_state *ps, const char *database_name_str,
                                         const char *semaphore_name_str)
{
    PRINT_STACK_TRACE(ps->tracer);
    
    if (semaphore_name_str)
    {
        ps->db_sem_name = mm_strdup(semaphore_name_str, ps->mm);
        if (!ps->db_sem_name)
        {
            SET_ERROR(ps->err);
            return -1;
        }
        ps->db_sem = sem_open(semaphore_name_str, O_CREAT, S_IRUSR | S_IWUSR, 1);
        if (ps->db_sem == SEM_FAILED)
        {
            SET_ERROR(ps->err);
            sem_unlink(semaphore_name_str);
            return -1;
        }
    }
    
    if (!database_name_str)
    {
        // NOLINTNEXTLINE(concurrency-mt-unsafe) : No threads here
        (void) fprintf(stdout, USAGE_MESSAGE);
        return -1;
    }
    
    ps->db_name = mm_strdup(database_name_str, ps->mm);
    if (!ps->db_name)
    {
        SET_ERROR(ps->err);
        return -1;
    }
    
    return 0;
}

static int scan_database(struct program_state *ps)
{
    PRINT_STACK_TRACE(ps->tracer);
    DBM   *db;
    datum key;
    datum value;
    int   count;
    
    count = 0;
    
    if (ps->db_sem && sem_wait(ps->db_sem) == -1)
    {
        SET_ERROR(ps->err);
        return -1;
    }
    // NOLINTBEGIN(concurrency-mt-unsafe) : Protected
    db = dbm_open(ps->db_name, DB_FLAGS, DB_FILE_MODE);
    if (db == (DBM *) 0)
    {
        SET_ERROR(ps->err);
        return -1;
    }
    key   = dbm_firstkey(db);
    value = dbm_fetch(db, key);
    if (!key.dptr && dbm_error(db))
    {
        print_db_error(db);
    }
    
    if (key.dptr)
    {
        // NOLINTNEXTLINE(concurrency-mt-unsafe): No threads here
        PRINT_ENTRY(key, value);
        ++count;
    }
    // Compare the display name to the name in the db
    while (key.dptr)
    {
        key   = dbm_nextkey(db);
        value = dbm_fetch(db, key);
        if (!key.dptr && dbm_error(db))
        {
            print_db_error(db);
            break;
        }
        if (key.dptr)
        {
            // NOLINTNEXTLINE(concurrency-mt-unsafe): No threads here
            PRINT_ENTRY(key, value);
            ++count;
        }
    }
    
    if (count)
    {
        (void) fprintf(stdout, "No records found; check the path to the database to ensure it is correct.\n");
    }
    dbm_close(db);
    // NOLINTEND(concurrency-mt-unsafe) : Protected
    if (ps->db_sem)
    {
        sem_post(ps->db_sem);
    }
    
    return 0;
}

void destroy_program_state(struct program_state *ps)
{
    mm_free(ps->mm, ps->db_name);
    sem_close(ps->db_sem);
    sem_unlink(ps->db_sem_name);
    mm_free(ps->mm, ps->db_sem_name);
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
