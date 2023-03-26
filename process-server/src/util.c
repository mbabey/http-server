#include <util.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BASE_10 10

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

void to_lower(char * s) {
    for(int i = 0; s[i]; i++){
        s[i] = (char)tolower(s[i]);
    }
}

struct http_header * get_header(const char * key, struct http_header ** headers, const size_t * num_headers) {
    for (size_t i = 0; i < *num_headers; i++) {
        if(strcmp(key, headers[i]->key) == 0) {
            return headers[i];
        }
    }
    return NULL;
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
