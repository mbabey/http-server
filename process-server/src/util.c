#include <util.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int write_fully(int fd, void * data, size_t size) {
    ssize_t result;
    ssize_t nwrote = 0;

    while (nwrote < (ssize_t)size) {
        result = send(fd, ((char*)data)+nwrote, size - nwrote, MSG_NOSIGNAL);
        if (result == -1) {
            perror("writing fully");
            return -1;
        }
        nwrote += result;
    }

    return 0;
}

int read_fully(int fd, void * data, size_t size) {
    ssize_t result;
    ssize_t nread = 0;

    while (nread < (ssize_t)size) {
        result = read(fd, ((char*)data)+nread, size-nread);
        if (result == -1) {
            perror("reading fully");
            return -1;
        }
        nread += result;
    }

    return 0;
}

char * set_string(char **str, const char *new_str)
{
    size_t buf = strlen(new_str) + 1;
    
    if (!*str) // Double negative: true if the string is NULL
    {
        if((*str = (char *) malloc(buf)) == NULL)
        {
            return NULL;
        }
    } else
    {
        if ((*str = (char *) realloc(*str, buf)) == NULL)
        {
            return NULL;
        }
    }
    
    strcpy(*str, new_str);
    
    return *str;
}

char *prepend_string(char **str, const char *prefix)
{
    char *new_str;
    size_t str_buf = strlen(*str) + 1;
    size_t prefix_size = strlen(prefix);
    
    if ((new_str = (char *) calloc(str_buf + prefix_size, sizeof(char))) == NULL)
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
    char *new_str;
    size_t str_buf = strlen(*str) + 1;
    size_t suffix_size = strlen(suffix);
    
    if ((new_str = (char *) calloc(str_buf + suffix_size, sizeof(char))) == NULL)
    {
        return NULL;
    }
    
    strcat(new_str, *str);
    strcat(new_str, suffix);
    set_string(str, new_str);
    
    free(new_str);
    
    return *str;
}
