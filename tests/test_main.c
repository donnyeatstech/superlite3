#include "../src/os/backend.h"
#include "../src/os/os.h"

#include <errno.h>
#include <fcntl.h>
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

static int failures = 0;

#define check(actual, expected)                                                \
    do {                                                                       \
        if (actual != expected) {                                              \
            failures++;                                                        \
            LOG("FAIL %s != %s", #actual, #expected);                          \
        }                                                                      \
        LOG("SUCCESS %s == %s", #actual, #expected);                           \
    } while (0)
#define LOG(fmt, ...)                                                          \
    printf("%s: %d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)

void file_not_exists() {
    struct syscalls *prev = swap_backend(&test_syscalls);

    const char *fake_path = "/fake/path.c";
    struct stat *statbuf = {};

    int out = stat_file(fake_path, statbuf);
    check(out, -1);
    check(errno, ENOENT);

    swap_backend(prev);
}

void file_exists() {

    int dirfd = 1;
    const char *path = "fake/example1.txt";
    int fd_int = 1;
    int *out_fd = &fd_int;

    struct syscalls *prev = swap_backend(&test_syscalls);

    int out = create_file(dirfd, path, out_fd);
    check(out, 0);

    const char *fake_path = "fake/example1.txt";
    struct stat statbuf_real = {};
    struct stat *statbuf = &statbuf_real;

    out = stat_file(fake_path, statbuf);
    check(out, 0);
    swap_backend(prev);
}

void not_enough_dir_permissions() {

    int dirfd = 9;
    const char *path = "/fake/not_enough_dir_permissions.txt";
    int out_fd = 1;
    int *out_fd_ptr = &out_fd;

    struct syscalls *prev = swap_backend(&test_syscalls);

    int out = create_file(dirfd, path, out_fd_ptr);
    check(out, -1);
    check(errno, EACCES);

    swap_backend(prev);
}

void file_exists_create_fails() {
    int dirfd = 1;
    const char *path = "fake/example1.txt";
    int fd_int = 1;
    int *out_fd = &fd_int;

    struct syscalls *prev = swap_backend(&test_syscalls);

    int out = create_file(dirfd, path, out_fd);
    check(out, -1);
    check(errno, EEXIST);

    swap_backend(prev);
}

void open_file_invalid_file_path() {

    const char *path = "some/fake/path.txt";
    struct syscalls *prev = swap_backend(&test_syscalls);
    int out_fd = 2;
    int *out_ptr = &out_fd;
    int err = open_file(AT_FDCWD, path, O_CREAT | O_RDONLY, 0, out_ptr);
    check(err, -1);
    check(errno, ENOENT);
    swap_backend(prev);
}

int main() {
    file_not_exists();
    file_exists();
    not_enough_dir_permissions();
    file_exists_create_fails();
    LOG("failures: %d", failures);
    // open_file_invalid_file_path();
}
