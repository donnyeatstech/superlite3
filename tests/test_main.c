#include "../src/os/backend.h"
#include "../src/os/os.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern struct syscalls test_syscalls;

static int failures = 0;

#define check(actual, expected)                                                \
    do {                                                                       \
        if (actual != expected) {                                              \
            failures++;                                                        \
            LOG("FAIL %s != %s", #actual, #expected);                          \
        } else {                                                               \
            LOG("SUCCESS %s == %s", #actual, #expected);                       \
        }                                                                      \
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

void file_exists(int *out_fd) {

    int dirfd = 1;
    const char *path = "fake/example1.txt";

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

void open_file_permission_denied() {
    const char *path = "some/fake/permission_denied.txt";
    struct syscalls *prev = swap_backend(&test_syscalls);
    int out_fd = 2;
    int *out_ptr = &out_fd;
    int err = open_file(AT_FDCWD, path, O_CREAT | O_RDONLY, 0, out_ptr);
    check(err, -1);
    check(errno, EACCES);
    swap_backend(prev);
}

void open_file_success() {
    const char *path = "some/fake/valid_file.txt";
    struct syscalls *prev = swap_backend(&test_syscalls);
    int out_fd = 2;
    int *out_ptr = &out_fd;
    int err = open_file(AT_FDCWD, path, O_CREAT | O_RDONLY, 0, out_ptr);
    check(err, 0);
    check(out_fd, 2);
    swap_backend(prev);
}

void read_file_reads_all_written_bytes_in_file(int fd) {

    struct syscalls *prev = swap_backend(&test_syscalls);
    uint8_t write_buffer[512];
    for (int i = 0; i < (int)sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    write_buffer[240] = 0x00;
    int err = write_file(fd, write_buffer, sizeof(write_buffer), 0);
    check(err, 0);

    uint8_t read_buffer[512];
    for (int i = 0; i < (int)sizeof(read_buffer); i++) {
        read_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    read_buffer[240] = 0x00;
    err = read_file(fd, read_buffer, sizeof(read_buffer), 0);
    check(err, 0);
    err = memcmp(write_buffer, read_buffer, sizeof(write_buffer));
    check(err, 0);
    swap_backend(prev);
}

void read_file_unexpected_eof(int fd) {
    struct syscalls *prev = swap_backend(&test_syscalls);
    uint8_t write_buffer[512];
    for (int i = 0; i < (int)sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    write_buffer[240] = 0x00;
    int err = write_file(fd, write_buffer, sizeof(write_buffer), 512);
    check(err, 0);

    uint8_t read_buffer[512];
    for (int i = 0; i < (int)sizeof(read_buffer); i++) {
        read_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    read_buffer[240] = 0x00;
    err = read_file(fd, read_buffer, sizeof(read_buffer), 600);
    check(err, -1);
    swap_backend(prev);
}

int main() {
    file_not_exists();
    int out = 1;
    int *out_fd = &out;
    file_exists(out_fd);
    not_enough_dir_permissions();
    file_exists_create_fails();
    LOG("STAT & CREATE failures: %d", failures);
    failures = 0;
    open_file_invalid_file_path();
    open_file_permission_denied();
    open_file_success();
    read_file_reads_all_written_bytes_in_file(*out_fd);
    read_file_unexpected_eof(*out_fd);
    LOG("OPEN & READ & CLOSE failures: %d", failures);
    if (failures > 0) {
        return -1;
    }
    return 0;
}
