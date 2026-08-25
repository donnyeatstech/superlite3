
#include "backend.h"
#include <sys/stat.h>
#include <unistd.h>

static int test_openat(int fd, const char *buf, int oflag, mode_t mode) {
    return 0;
}

static int test_stat(const char *restrict path, struct stat *restrict statbuf) {
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

static struct syscalls test_syscalls = {.openat = test_openat,
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
