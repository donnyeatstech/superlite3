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

typedef enum { OK = 0, OS_IOERR, OS_BUSY, OS_NOTFOUND, OS_EXISTS } os_rt;

os_rt open_file(int fd, const char *path, int oflag, ...);
