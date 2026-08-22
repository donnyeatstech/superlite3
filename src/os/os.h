#include <sys/stat.h>
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

int stat_file(const char *restrict path, struct stat *restrict statbuf);

int open_file(int fd, const char *path, int oflag, int *out_fd);

int read_file(int fd, void *buf, size_t nbyte, off_t offset);

int create_file(int dirfd, const char *path, int *out_fd);

int write_file(int fd, const void *buf, size_t nbytes, off_t offset);

int close_file(int fd);
