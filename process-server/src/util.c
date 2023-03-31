#include <util.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "manager.h"

#define BASE_10 10
#define HTTP_TIME_FORMAT "%a, %d %b %Y %H:%M:%S %Z"

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

char * litlittok(char * str, char * sep) {
    // shameless copy of https://stackoverflow.com/questions/59770865/strtok-c-multiple-chars-as-one-delimiter
    static char *string;
    if (str != NULL) {
        string = str;
    }

    if (string == NULL) {
        return string;
    }

    char *end = strstr(string, sep);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(sep);
    return temp;
}

char * h_litlittok(char * str, char * sep) {
    // shameless copy of https://stackoverflow.com/questions/59770865/strtok-c-multiple-chars-as-one-delimiter
    static char *string;
    if (str != NULL) {
        string = str;
    }

    if (string == NULL) {
        return string;
    }

    char *end = strstr(string, sep);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(sep);
    return temp;
}

char * rl_litlittok(char * str, char * sep) {
    // shameless copy of https://stackoverflow.com/questions/59770865/strtok-c-multiple-chars-as-one-delimiter
    static char *string;
    if (str != NULL) {
        string = str;
    }

    if (string == NULL) {
        return string;
    }

    char *end = strstr(string, sep);
    if (end == NULL) {
        char *temp = string;
        string = NULL;
        return temp;
    }

    char *temp = string;

    *end = '\0';
    string = end + strlen(sep);
    return temp;
}

void to_lower(char * s) {
    for(int i = 0; s[i]; i++){
        s[i] = (char)tolower(s[i]);
    }
}

struct http_header * get_header(const char * key, struct http_header ** headers, const size_t num_headers) {
    for (size_t i = 0; i < num_headers; i++) {
        if(strcmp(key, headers[i]->key) == 0) {
            return headers[i];
        }
    }
    return NULL;
}

struct http_header *set_header(struct core_object *co, const char *key, const char *value)
{
    PRINT_STACK_TRACE(co->tracer);
    
    struct http_header *header;
    
    header = mm_malloc(sizeof(struct http_header), co->mm);
    if (!header)
    {
        SET_ERROR(co->err);
        return NULL;
    }
    
    header->key = mm_strdup(key, co->mm);
    header->value = mm_strdup(value, co->mm);
    if (!header->key || !header->value)
    {
        SET_ERROR(co->err);
        return NULL;
    }
    
    return header;
}

void destroy_http_header(struct http_header * header, struct core_object * co) {
    if (header->key) {
        mm_free(co->mm, header->key);
    }
    if (header->value) {
        mm_free(co->mm, header->value);
    }
    mm_free(co->mm, header);
}

void free_all_headers(struct core_object *co, struct http_header **headers)
{
    PRINT_STACK_TRACE(co->tracer);
    
    for (; *headers; headers++)
    {
        destroy_http_header(*headers, co);
    }
    
    mm_free(co->mm, headers);
}

void free_http_data(struct core_object *co, struct http_header **headers, char *entity_body)
{
    PRINT_STACK_TRACE(co->tracer);
    
    free_all_headers(co, headers);
    mm_free(co->mm, entity_body);
}

size_t strtosize_t(char * str) {
    size_t val;
    val = strtoul(str, NULL, BASE_10);
    if (val == 0) {
        perror("strtoul parsing string");
        return 0;
    }
    return val;
}

char * trim_whitespace(char * str) {
    size_t size;
    char *end;

    size = strlen(str);

    if (!size)
        return str;

    end = str + size - 1;
    while (end >= str && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*str && isspace(*str))
        str++;

    return str;
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
    if (set_string(str, new_str) == NULL)
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


int http_time_now(char dst[HTTP_TIME_LEN]) {
    memset(dst, 0, HTTP_TIME_LEN);
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    if (strftime(dst, HTTP_TIME_LEN, HTTP_TIME_FORMAT, &tm) == 0) {
        (void) fprintf(stderr, "error getting current HTTP time\n");
        return -1;
    }
    return 0;
}

time_t http_time_to_time_t(char http_time[HTTP_TIME_LEN]) {
    struct tm tm;
    time_t t;

    if (strptime(http_time, HTTP_TIME_FORMAT, &tm) == NULL) {
        (void) fprintf(stderr, "error converting HTTP time to time_t time: converting t to tm struct\n");
        return -1;
    }

    t = mktime(&tm);
    if (t == -1) {
        (void) fprintf(stderr, "error converting HTTP time to time_t time: converting t to time_t\n");
        return -1;
    }

    return t;
}

int compare_http_time(char t1_str[HTTP_TIME_LEN], char t2_str[HTTP_TIME_LEN]) {
    time_t t1, t2;
    double diff;

    t1 = http_time_to_time_t(t1_str);
    if (t1 == -1) {
        return -1;
    }
    t2 = http_time_to_time_t(t2_str);
    if (t2 == -1) {
        return -1;
    }
    diff = difftime(t1, t2);
    if (diff > 0) {
        return 1;
    }
    return 0;
}
