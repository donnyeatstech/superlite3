#include "os.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int open_file(int dirfd, const char *path, int *out_fd) {
    int file_fd = openat(dirfd, path, O_RDONLY);
    if (file_fd == -1) {
        printf("open_file error: %s", errno);
        return errno;
    }
    *out_fd = file_fd;
    return 0;
}

int create_file(int dirfd, const char *path) {
    int file_fd = open_file(dirfd, path, O_RDONLY);
    if (file_fd != -1) {
        printf("create_file error, file exists: %s", errno);
        return -1;
    }

    file_fd = openat(dirfd, path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (file_fd < 0) {
        printf("create_file error: %s", errno);
        return -errno;
    }

    return file_fd;
}

ssize_t write_file(int dirfd, const char *path, size_t count, const void *buf) {

    int out = write(dirfd, buf, count);
    if (out < 0) {
        printf("write_file error: %s", errno);
        return errno;
    }

    return 0;
}

int close_file(int fd) {
    int err = close(fd);

    if (err < 0) {
        printf("close_file error: %s", errno);
        return errno;
    }

    return 0;
}
