#include "../src/os/backend.h"
#include "../src/os/os.h"
#include "../tests/os/test_backend.h"

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
    printf("%s: %d %s() " fmt "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)

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

void read_file_eintr_but_success(int fd) {
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
        read_buffer[i] = (uint8_t)(i * 31) + 8;
    }
    read_buffer[241] = 0x00;
    arm_eintr(3);
    err = read_file(fd, read_buffer, sizeof(read_buffer), 0);
    check(err, 0);
    check(get_eintr_cnt(), 0);
    err = memcmp(write_buffer, read_buffer, sizeof(write_buffer));
    check(err, 0);
    swap_backend(prev);
}

void read_file_at_offset_get_correct_bytes(int fd) {
    struct syscalls *prev = swap_backend(&test_syscalls);
    uint8_t write_buffer[512];
    for (int i = 0; i < (int)sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    write_buffer[240] = 0x00;
    int err = write_file(fd, write_buffer, sizeof(write_buffer), 0);
    check(err, 0);
    uint8_t read_buffer[256];
    read_buffer[240] = 0x00;
    err = read_file(fd, read_buffer, sizeof(read_buffer), 256);
    check(err, 0);
    err = memcmp(write_buffer + 256, read_buffer, sizeof(read_buffer));
    check(err, 0);
    swap_backend(prev);
}

void close_file_succeeds(int fd) {
    struct syscalls *prev = swap_backend(&test_syscalls);
    uint8_t read_buffer[256];
    read_buffer[240] = 0x00;
    int err = read_file(fd, read_buffer, sizeof(read_buffer), 0);
    check(err, 0);
    err = close_file(fd);
    check(err, 0);
    swap_backend(prev);
}

void close_file_invalid_fd() {
    struct syscalls *prev = swap_backend(&test_syscalls);
    uint8_t read_buffer[256];
    read_buffer[240] = 0x00;
    int err = close_file(999);
    check(err, -1);
    check(errno, EBADF);
    swap_backend(prev);
}

void write_fails_permission_denied() {
    struct syscalls *prev = swap_backend(&test_syscalls);
    int dirfd = 1;
    const char *path = "fake/write_fails_permission_denied.txt";

    int out_fd = 6;

    int err = create_file(dirfd, path, &out_fd);
    check(err, 0);

    uint8_t write_buffer[512];

    err = write_file(out_fd, write_buffer, sizeof(write_buffer), 0);
    check(err, -1);
    check(errno, EACCES);
    swap_backend(prev);
}

void write_file_short_write() {
    struct syscalls *prev = swap_backend(&test_syscalls);
    int dirfd = 1;
    const char *path = "fake/write_fails_short_write.txt";

    int out_fd = 7;

    int err = create_file(dirfd, path, &out_fd);
    errno = 0;
    check(err, 0);

    uint8_t write_buffer[512];

    err = write_file(out_fd, write_buffer, sizeof(write_buffer), 0);
    check(err, -1);
    check(errno, 0);
    swap_backend(prev);
}

void write_file_eintr_but_success() {

    struct syscalls *prev = swap_backend(&test_syscalls);
    int dirfd = 1;
    const char *path = "fake/write_eintr_but_success.txt";
    int out_fd = 7;
    int err = create_file(dirfd, path, &out_fd);
    check(err, 0);

    uint8_t write_buffer[512];
    for (int i = 0; i < (int)sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    write_buffer[240] = 0x00;
    arm_eintr(3);
    err = write_file(out_fd, write_buffer, sizeof(write_buffer), 0);
    check(err, 0);
    check(get_eintr_cnt(), 0);

    swap_backend(prev);
}

void write_file_at_offset_ensure_correct_bytes() {

    struct syscalls *prev = swap_backend(&test_syscalls);
    int dirfd = 1;
    const char *path = "fake/write_at_an_offset.txt";
    int out_fd = 8;
    int err = create_file(dirfd, path, &out_fd);
    check(err, 0);

    uint8_t write_buffer[512];
    for (int i = 0; i < (int)sizeof(write_buffer); i++) {
        write_buffer[i] = (uint8_t)(i * 31) + 7;
    }
    write_buffer[240] = 0x00;

    // base bytes written to fd
    err = write_file(out_fd, write_buffer, sizeof(write_buffer), 0);
    check(err, 0);

    // memcmp written bytes and intended bytes for whole buffer
    uint8_t read_buffer[512];
    read_buffer[240] = 0x00;
    err = read_file(out_fd, read_buffer, sizeof(read_buffer), 0);
    check(err, 0);
    err = memcmp(write_buffer, read_buffer, sizeof(read_buffer));
    check(err, 0);

    // bytes written to an offset
    uint8_t small_buffer[64];
    for (int i = 0; i < (int)sizeof(small_buffer); i++) {
        small_buffer[i] = (uint8_t)(i * 31) + 8;
    }
    err = write_file(out_fd, small_buffer, sizeof(small_buffer), 256);
    check(err, 0);

    // memcmp written offset bytes, and bytes before and after offset, offset +
    // nbytes
    err = read_file(out_fd, read_buffer, sizeof(read_buffer), 0);
    check(err, 0);
    err = memcmp(
        small_buffer, read_buffer + 256,
        sizeof(small_buffer)); // compare 64 bytes at an offset of 256 bytes
    check(err, 0);

    err = memcmp(write_buffer, read_buffer,
                 (unsigned long)256); // compare first 256 bytes
    check(err, 0);
    err = memcmp(
        write_buffer + (unsigned long)(256 + 64),
        read_buffer + (unsigned long)(256 + 64),
        (unsigned long)(256 - 64)); // compare 192 bytes at an offset of 320

    check(err, 0);
    swap_backend(prev);
}

void fsync_success() {

    struct syscalls *prev = swap_backend(&test_syscalls);
    int dirfd = 1;
    const char *path = "fake/fsync_success.txt";
    int out_fd = 9;
    int err = create_file(dirfd, path, &out_fd);
    check(err, 0);

    err = fsync_file(out_fd);
    check(err, 0);

    swap_backend(prev);
}

void fsync_failure() {

    struct syscalls *prev = swap_backend(&test_syscalls);
    int err = fsync_file(10);
    check(err, -1);
    check(errno, EBADF);

    swap_backend(prev);
}

int main() {
    file_not_exists();
    int out = 1;
    int *out_fd = &out;
    file_exists(out_fd);
    not_enough_dir_permissions();
    file_exists_create_fails();
    LOG("STAT & CREATE failures: %d\n", failures);

    open_file_invalid_file_path();
    open_file_permission_denied();
    open_file_success();
    read_file_reads_all_written_bytes_in_file(*out_fd);
    read_file_unexpected_eof(*out_fd);
    read_file_eintr_but_success(*out_fd);
    read_file_at_offset_get_correct_bytes(*out_fd);
    close_file_succeeds(*out_fd);
    close_file_invalid_fd();
    LOG("OPEN & READ & CLOSE failures: %d\n", failures);

    write_fails_permission_denied();
    write_file_short_write();
    write_file_eintr_but_success();
    write_file_at_offset_ensure_correct_bytes();
    LOG("OPEN & WRITE & CLOSE failures: %d\n", failures);

    fsync_success();
    fsync_failure();

    if (failures > 0) {
        return -1;
    }

    LOG("TOTAL failures: %d\n", failures);
    return 0;
}

/*
 *
Group A — stat_file + create_file (existence & creation)

1. File doesn't exist → stat_file reports it doesn't exist (ENOENT) done
2. File doesn't exist → create_file succeeds -> stat_file confirms it exists
done
3. File doesn't exist, permission denied on the containing dir → create_file
fails, EACCES done
4. [new] File already exists → create_file fails, EEXIST — this is the entire
reason O_EXCL is set; currently untested done

Group B — open_file + read_file + close_file

6. open_file: path doesn't exist → ENOENT done
7. open_file: path exists, permission denied → EACCES done
8. open_file: path exists → succeeds, valid fd returned done
9. read_file: reads back exactly the bytes that were written (round-trip
integrity — this is what "corrupt data" from your notes becomes: prove the read
path never mangles bytes) done
10. [new] read_file: short read / unexpected EOF (fewer bytes available than
requested) → returns -1 — the exact bug fixed earlier in this project; this is
its regression test done
11. [new] read_file: one EINTR then success → retry loop completes, correct
bytes returned done
12. [new] read_file: read at a non-zero offset → correct bytes, correct position
(not just offset 0) done
13. close_file: succeeds done
14. close_file: fails (e.g. bad fd, EBADF) done

Group C — open_file + write_file + close_file

Sets up its own file at the top of this group's tests — does not depend on Group
A having run. (Reuses Group B's open/close cases 6-8, 13-14; not re-listed.)

15. write_file: succeeds, and the written bytes read back correctly (pairs with
case 9) done
16. write_file: fails, permission denied done
17. [new] write_file: returns 0 bytes / short write → returns -1 — write-side
counterpart to case 10 done
18. [new] write_file: one EINTR then success done
19. [new] write_file at a non-zero offset → lands at the correct position,
doesn't clobber adjacent bytes (this is your bottom-of-page note, promoted to
its own case instead of a general reminder) DONNEEEEEE

Group D — open_file + fsync_file + close_file

Reuses Group B's open/close cases (6-8, 13-14).

20. [new] fsync_file: succeeds
21. [new] fsync_file: fails (e.g. EBADF on a bad fd)
 *
 *
 *
 * */
