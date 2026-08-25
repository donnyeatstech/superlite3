
#include "backend.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static int real_openat(int fd, const char *buf, int oflag, mode_t mode) {
    return openat(fd, buf, oflag, mode);
}

static int real_stat(const char *restrict path, struct stat *restrict statbuf) {
    return stat(path, statbuf);
}

static ssize_t real_pread(int fd, void *buf, size_t nbytes, off_t offset) {
    return pread(fd, buf, nbytes, offset);
}

static ssize_t real_pwrite(int fd, const void *buf, size_t nbytes,
                           off_t offset) {
    return pwrite(fd, buf, nbytes, offset);
}

static int real_close(int fd) { return close(fd); }

static int real_fsync(int fd) { return fsync(fd); }

static struct syscalls real_syscalls = {.openat = real_openat,
                                        .stat = real_stat,
                                        .pread = real_pread,
                                        .pwrite = real_pwrite,
                                        .close = real_close,
                                        .fsync = real_fsync};

struct syscalls *active_backend = &real_syscalls;
