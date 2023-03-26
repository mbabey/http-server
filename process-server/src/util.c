#include <util.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
