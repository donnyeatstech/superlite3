
#include "test_backend.h"
#include <sys/stat.h>
#include <unistd.h>

int openat(int fd, const char *buf, mode_t mode) { return 0; }

int stat(const char *restrict path, struct stat *restrict statbuf) { return 0; }

ssize_t pread(int fd, void *buf, size_t nbytes, off_t offset) { return 0; }

ssize_t pwrite(int fd, const void *buf, size_t nbytes, off_t offset) {
    return 0;
}

int close(int fd) { return 0; }

int fsync(int fd) { return 0; }
