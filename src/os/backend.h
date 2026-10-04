#ifndef BACKEND_DECLARATION
#define BACKEND_DECLARATION

#include <sys/stat.h>
#include <sys/types.h>

struct syscalls {
    int (*openat)(int, const char *, int, mode_t);
    int (*stat)(const char *, struct stat *);
    ssize_t (*pread)(int, void *, size_t, off_t);
    ssize_t (*pwrite)(int, const void *, size_t, off_t);
    int (*close)(int);
    int (*fsync)(int);
};

extern struct syscalls *active_backend;

struct syscalls *swap_backend(struct syscalls *next);

#endif
