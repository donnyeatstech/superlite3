#include "os.h"
#include "backend.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int stat_file(const char *restrict path, struct stat *restrict statbuf) {
    int out = active_backend->stat(path, statbuf);

    if (out < 0) {
        printf("stat_file error: %d\n", errno);
        return errno;
    }

    return 0;
}

int open_file(int dirfd, const char *path, int oflag, int mode, int *out_fd) {
    int file_fd;
    do {

        file_fd = active_backend->openat(dirfd, path, oflag, mode);
    } while (file_fd == -1 && errno == EINTR);
    if (file_fd == -1) {
        printf("open_file error: %d\n", errno);
        return errno;
    }
    *out_fd = file_fd;
    return 0;
}

int read_file(int fd, void *buf, size_t nbytes, off_t offset) {
    ssize_t out_bytes;
    char *ptr = (char *)buf;
    size_t bytes_read = 0;

    while (bytes_read < nbytes) {
        out_bytes = active_backend->pread(
            fd, ptr + bytes_read, nbytes - bytes_read, offset + bytes_read);
        if (out_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("read_file error: %d\n", errno);
            return errno;
        }
        if ((size_t)out_bytes == 0) {
            printf("read_file unexpected EOF. expected %zu bytes\n", nbytes);
            return -1;
        }

        bytes_read += out_bytes;
    }

    return 0;
}

int create_file(int dirfd, const char *path, int *out_fd) {
    int file_fd;
    do {
        file_fd = active_backend->openat(dirfd, path,
                                         O_WRONLY | O_CREAT | O_EXCL, 0600);
    } while (file_fd == -1 && errno == EINTR);
    if (file_fd < 0) {
        printf("create_file error: %d\n", errno);
        return errno;
    }
    *out_fd = file_fd;
    return 0;
}

int write_file(int fd, const void *buf, size_t nbytes, off_t offset) {
    ssize_t out_bytes;
    const char *ptr = (char *)buf;
    size_t bytes_read = 0;
    while (bytes_read < nbytes) {
        out_bytes = active_backend->pwrite(
            fd, ptr + bytes_read, nbytes - bytes_read, offset + bytes_read);
        if (out_bytes < 0) {
            if (errno == EINTR) {
                continue;
            }
            printf("write_file error: %d\n", errno);
            return errno;
        }

        if ((size_t)out_bytes == 0) {
            printf("write_file returned 0 bytes. expected %zu bytes\n", nbytes);
            return -1;
        }

        bytes_read += out_bytes;
    }

    return 0;
}

int fsync_file(int fd) {
    int err;
    do {

        err = active_backend->fsync(fd);
    } while (err == -1 && errno == EINTR);
    if (err < 0) {
        printf("fsync_file error: %d\n", errno);
        return errno;
    }
    return 0;
}

int close_file(int fd) {
    int err = active_backend->close(fd);

    if (err < 0) {
        printf("close_file error: %d\n", errno);
        return errno;
    }

    return 0;
}
