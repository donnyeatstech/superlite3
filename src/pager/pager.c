#include "pager.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

os_rt open_file(int fd, const char *path, int oflag, ...) {
    int file_fd = openat(fd, path, oflag);
    if (file_fd == -1) {
        return OS_NOTFOUND;
    }

    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    close(file_fd);

    return OK;
}
