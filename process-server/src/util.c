#include <util.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define VERSION_START_INDEX 2

#define WR_DIR_FLAGS (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

/**
 * version_file
 * <p>
 * Add a version number to a file name, if necessary.
 * </p>
 * @param save_str - char **: file name to which a version number will be added.
 */
static int version_file(char **save_str);

/**
 * create_file_ver_suffix
 * <p>
 * Create a string in the format "-v2".
 * </p>
 * @param file_ver_suffix - char **: pointer to string to hold the suffix
 * @param v_num - int: the version number to put in the suffix
 */
static int create_file_ver_suffix(char **file_ver_suffix, int v_num);

int write_fully(int fd, void *data, size_t size)
{
    ssize_t result;
    ssize_t nwrote = 0;
    
    while (nwrote < (ssize_t) size)
    {
        result = send(fd, ((char *) data) + nwrote, size - nwrote, MSG_NOSIGNAL);
        if (result == -1)
        {
            perror("writing fully");
            return -1;
        }
        nwrote += result;
    }
    
    return 0;
}

int read_fully(int fd, void *data, size_t size)
{
    ssize_t result;
    ssize_t nread = 0;
    
    while (nread < (ssize_t) size)
    {
        result = read(fd, ((char *) data) + nread, size - nread);
        if (result == -1)
        {
            perror("reading fully");
            return -1;
        }
        nread += result;
    }
    
    return 0;
}

char *set_string(char **str, const char *new_str)
{
    size_t buf = strlen(new_str) + 1;
    
    if (!*str) // Double negative: true if the string is NULL
    {
        *str = (char *) malloc(buf);
        if (*str == NULL)
        {
            return NULL;
        }
    } else
    {
        *str = (char *) realloc(*str, buf);
        if (*str == NULL)
        {
            return NULL;
        }
    }
    
    strcpy(*str, new_str);
    
    return *str;
}

char *prepend_string(char **str, const char *prefix)
{
    char   *new_str;
    size_t str_buf     = strlen(*str) + 1;
    size_t prefix_size = strlen(prefix);
    
    new_str = (char *) calloc(str_buf + prefix_size, sizeof(char));
    if (new_str == NULL)
    {
        return NULL;
    }
    
    strcat(new_str, prefix);
    strcat(new_str, *str);
    set_string(str, new_str);
    
    free(new_str);
    
    return *str;
}

char *append_string(char **str, const char *suffix)
{
    char   *new_str;
    size_t str_buf     = strlen(*str) + 1;
    size_t suffix_size = strlen(suffix);
    
    new_str = (char *) calloc(str_buf + suffix_size, sizeof(char));
    if (new_str == NULL)
    {
        return NULL;
    }
    
    strcat(new_str, *str);
    strcat(new_str, suffix);
    if (set_string(str, new_str) == NULL) // TODO: When does this point to deallocated memory?
    {
        return NULL;
    }
    
    free(new_str);
    
    return *str;
}

char * create_dir_str(char **save_dir, const char *wr_dir,
                      const char *client_addr_str) // NOLINT(bugprone-easily-swappable-parameters)
{
    if (set_string(save_dir, wr_dir) == NULL)
    {
        return NULL;
    }
    if (append_string(save_dir, "/") == NULL)
    {
        return NULL;
    }
    if (append_string(save_dir, client_addr_str) == NULL)
    {
        return NULL;
    }
    
    return *save_dir;
}

int create_dir(const char *save_dir)
{
    char   *path;
    size_t len;
    
    len       = strlen(save_dir);
    path = (char *) calloc(len + 1, sizeof(char));
    if (path == NULL)
    {
        return -1;
    }
    
    for (size_t index = 0; index < len; ++index)
    {
        *(path + index) = *(save_dir + index);
        if (index < len && (*(save_dir + index + 1) == '/' || *(save_dir + index + 1) == '\0'))
        {
            if (access(path, F_OK) != 0)
            {
                if ((mkdir(path, WR_DIR_FLAGS)) == -1)
                {
                    return -1;
                }
            }
        }
    }
    
    free(path);
    
    return 0;
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
    
    version_file(&save_file_name);
    
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

static int version_file(char **save_str)
{
    int  v_num            = VERSION_START_INDEX;
    char *file_ver_suffix = NULL;
    char *save_str_cpy    = NULL;
    char *ext             = NULL;
    char *ext_ptr;
    
    if (set_string(&save_str_cpy, *save_str) == NULL)
    {
        return -1;
    }
    
    while (access(save_str_cpy, F_OK) == 0) // does exist
    {
        if (v_num > VERSION_START_INDEX)
        {
            if (set_string(&save_str_cpy, *save_str) == NULL)
            {
                return -1;
            }
        }
        
        ext_ptr = strrchr(save_str_cpy, '.');
        if (set_string(&ext, ext_ptr) == NULL)
        {
            return -1;
        }
        
        if (create_file_ver_suffix(&file_ver_suffix, v_num) == -1)
        {
            return -1;
        }
        
        *ext_ptr = '\0';
        if (append_string(&save_str_cpy, file_ver_suffix) == NULL)
        {
            return -1;
        }
        if (append_string(&save_str_cpy, ext) == NULL)
        {
            return -1;
        }
        
        ++v_num;
    }
    if (set_string(save_str, save_str_cpy) == NULL)
    {
        return -1;
    }
    
    free(file_ver_suffix);
    free(save_str_cpy);
    free(ext);
    
    return 0;
}

static int create_file_ver_suffix(char **file_ver_suffix, int v_num)
{
    int  v_num_len;
    char *v_num_suffix;
    
    v_num_len = snprintf(NULL, 0, "%d", v_num);
    if (v_num_len < 0)
    {
        return -1;
    }
    v_num_suffix = (char *) calloc(v_num_len + 1, sizeof(char));
    if (v_num_suffix == NULL)
    {
        free(v_num_suffix);
        return -1;
    }
    if (snprintf(v_num_suffix, v_num_len + 1, "%d", v_num) < 0)
    {
        free(v_num_suffix);
        return -1;
    }
    
    if (set_string(file_ver_suffix, "-v") == NULL)
    {
        free(v_num_suffix);
        return -1;
    }
    if (append_string(file_ver_suffix, v_num_suffix) == NULL)
    {
        return -1;
    }
    
    free(v_num_suffix);
    
    return 0;
}
