#include "../src/os/backend.h"
#include "../src/os/os.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>

extern struct syscalls test_syscalls;

/* file existence and creation:
* 1. File doesn't exist → stat_file reports it doesn't exist (ENOENT)
2. File exists → stat_file confirms it exists
3. File doesn't exist → create_file succeeds
4. File doesn't exist, permission denied on the containing dir → create_file
fails, EACCES
5. [new] File already exists → create_file fails, EEXIST — this is the entire
reason O_EXCL is set; currently untested
 * */

int file_not_exists() {
    struct syscalls *prev = swap_backend(&test_syscalls);

    const char *fake_path = "/fake/path.c";
    struct stat *statbuf = {};

    int out = stat_file(fake_path, statbuf);
    assert(out == -1);
    assert(errno == ENOENT);

    printf("[FILE NOT EXISTS] SUCCESS asserted ENOENT error code: %d\n", errno);
    swap_backend(prev);
    return 0;
}

int file_exists() {

    int dirfd = 1;
    const char *path = "fake/example1.txt";
    int fd_int = 1;
    int *out_fd = &fd_int;

    struct syscalls *prev = swap_backend(&test_syscalls);

    int out = create_file(dirfd, path, out_fd);
    assert(out == 0);

    printf("[FILE EXISTS > create_file] SUCCESS asserted no error code: %d\n",
           ENOENT);

    const char *fake_path = "fake/example1.txt";
    struct stat statbuf_real = {};
    struct stat *statbuf = &statbuf_real;

    out = stat_file(fake_path, statbuf);
    assert(out == 0);
    printf("[FILE EXISTS > stat_file] SUCCESS asserted no error code: %d\n",
           ENOENT);

    swap_backend(prev);
    return 0;
}

int not_enough_dir_permissions() {

    int dirfd = 9;
    const char *path = "/fake/not_enough_dir_permissions.txt";
    int out_fd = 1;
    int *out_fd_ptr = &out_fd;

    struct syscalls *prev = swap_backend(&test_syscalls);

    int out = create_file(dirfd, path, out_fd_ptr);
    assert(out == -1);
    assert(errno = EACCES);
    printf("[NOT ENOUGH DIR PERMISSIONS > CREATE_FILE] SUCCESS asserted  error "
           "code: %d\n",
           errno);

    swap_backend(prev);
    return 0;
}

int file_exists_create_fails() {
    int dirfd = 1;
    const char *path = "fake/example1.txt";
    int fd_int = 1;
    int *out_fd = &fd_int;

    struct syscalls *prev = swap_backend(&test_syscalls);

    int out = create_file(dirfd, path, out_fd);
    assert(out == -1);
    assert(errno == EEXIST);

    printf("[FILE EXISTS CREATE FAILS > create_file] SUCCESS asserted "
           "code: %d\n",
           errno);

    swap_backend(prev);
    return 0;
}

int main() {
    file_not_exists();
    file_exists();
    not_enough_dir_permissions();
    file_exists_create_fails();
    return 0;
}
