#ifndef TEST_BACKEND
#define TEST_BACKEND

#include <sys/types.h>

struct syscalls {
    int (*openat)(int, const char *, int, mode_t);
    int (*stat)(const char *, struct stat *);
    ssize_t (*pread)(int, void *, size_t, off_t);
    ssize_t (*pwrite)(int, const void *, size_t, off_t);
    int (*close)(int);
    int (*fsync)(int);
};

extern struct syscalls *ptr;

#endif TEST_BACKEND
