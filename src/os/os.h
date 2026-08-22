#include <unistd.h>
/*
 * Wrappers around syscalls that the code will require.
 * syscalls include:
 * 1. openat
 * 2. write
 * 3. close
 * 4. stat
 *
 *
 *
 */

int open_file(int fd, const char *path, int *out_fd);

int create_file(int dirfd, const char *path);

ssize_t write_file(int dirfd, const char *path, size_t count, const void *buf);

int close_file(int fd);
