
#include "../../src/os/backend.h"
#include <errno.h>
#include <stdio.h>

#define MAX_FAKE_FILES 30
static const char *fake_files[MAX_FAKE_FILES];
static int fake_file_count = 0;

int fake_file_exists(const char *path) {
    for (int i = 0; i <= fake_file_count; i++) {
        if (fake_files[i] == path) {
            return 1;
        }
    }
    return -1;
}

static int test_openat(int fd, const char *buf, int oflag, mode_t mode) {
    return 0;
}

static int test_stat(const char *restrict path, struct stat *restrict statbuf) {

    if (fake_file_exists(path) == -1) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

static ssize_t test_pread(int fd, void *buf, size_t nbytes, off_t offset) {
    return 0;
}

static ssize_t test_pwrite(int fd, const void *buf, size_t nbytes,
                           off_t offset) {
    return 0;
}

static int test_close(int fd) { return 0; }

static int test_fsync(int fd) { return 0; }

struct syscalls test_syscalls = {.openat = test_openat,
                                 .stat = test_stat,
                                 .pread = test_pread,
                                 .pwrite = test_pwrite,
                                 .close = test_close,
                                 .fsync = test_fsync};

struct syscalls *swap_backend(struct syscalls *next) {
    struct syscalls *prev = active_backend;
    active_backend = next;
    return prev;
}
