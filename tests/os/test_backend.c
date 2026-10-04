
#include "test_backend.h"
#include "../../src/os/backend.h"
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MAX_FAKE_FILES 30
#define FILE_BUFFER_CAP 4096
static const char *fake_files[MAX_FAKE_FILES];
struct fake_file_buffer {
    const char *path;
    int fd;
    uint8_t filebuffer[FILE_BUFFER_CAP];
    ssize_t len;
    off_t offset;
};

static int eintr_cnt = 0;
void arm_eintr(int n) { eintr_cnt = n; }
int get_eintr_cnt() { return eintr_cnt; }

static struct fake_file_buffer fake_file_buffers[MAX_FAKE_FILES];

static int fake_file_count = 0;

struct fake_file_buffer *fake_file_exists(const char *path, int fd) {
    if (fd != -1) {
        for (int i = 0; i < fake_file_count; i++) {
            if (fake_file_buffers[i].fd == fd) {
                return &fake_file_buffers[i];
            }
        }
    } else {
        for (int i = 0; i < fake_file_count; i++) {
            if (fake_file_buffers[i].path == path) {
                return &fake_file_buffers[i];
            }
        }
    }
    return NULL;
}

void add_fake_file(const char *path, int fd, void *buf, size_t len) {
    struct fake_file_buffer new_file = {};
    new_file.path = path;
    new_file.fd = fd;
    if (buf != NULL) {
        memcpy(new_file.filebuffer, buf, len);
    }
    new_file.len = len;
    fake_file_buffers[fake_file_count] = new_file;
    fake_file_count++;
}

static int test_openat(int fd, const char *path, int oflag, mode_t mode) {
    if (oflag ==
        (O_CREAT | O_RDONLY)) { // READ FILE AND CREATE IF DOESN'T EXIST
        if (strcmp(path, "some/fake/path.txt") == 0) {
            errno = ENOENT;
            return -1;
        }
        if (strcmp(path, "some/fake/permission_denied.txt") == 0) {
            errno = EACCES;
            return -1;
        }
        if (strcmp(path, "some/fake/valid_file.txt") == 0) {
            return 2;
        }
        return 0;
    } else if (oflag == (O_RDWR | O_CREAT | O_EXCL)) { // ONLY CREATE FILE
        if (fake_file_exists(path, -1) != NULL) {
            errno = EEXIST;
            return -1;
        }
        if (strcmp(path, "/fake/not_enough_dir_permissions.txt") == 0) {
            errno = EACCES;
            return -1;
        }
        int new_fd = fake_file_count + 3;
        add_fake_file(path, new_fd, NULL, -1);
        return new_fd;
    }
    return 0;
}

static int test_stat(const char *restrict path, struct stat *restrict statbuf) {

    if (fake_file_exists(path, -1) == NULL) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

static ssize_t test_pread(int fd, void *buf, size_t nbytes, off_t offset) {
    struct fake_file_buffer *file = fake_file_exists(NULL, fd);
    if (file == NULL) {
        errno = EBADF;
        return -1;
    }
    ssize_t bytes_avl = file->len - offset;
    if (bytes_avl < 0) {
        return 0;
    }
    if (eintr_cnt > 0) {
        eintr_cnt--;
        errno = EINTR;
        return -1;
    }
    size_t lim = (size_t)bytes_avl;
    lim = lim < nbytes ? lim : nbytes;
    memcpy(buf, file->filebuffer + offset, lim);
    return (ssize_t)lim;
}

static ssize_t test_pwrite(int fd, const void *buf, size_t nbytes,
                           off_t offset) {
    struct fake_file_buffer *file = fake_file_exists(NULL, fd);
    if (file == NULL) {
        errno = EBADF;
        return -1;
    }
    if (offset < 0) {
        errno = EINVAL;
        return -1;
    }
    if (offset + nbytes > FILE_BUFFER_CAP) {
        errno = EFBIG;
        return -1;
    }
    if (strcmp(file->path, "fake/write_fails_permission_denied.txt") == 0) {
        errno = EACCES;
        return -1;
    }
    if (strcmp(file->path, "fake/write_fails_short_write.txt") == 0) {
        return 0;
    }
    if (eintr_cnt > 0) {
        eintr_cnt--;
        errno = EINTR;
        return -1;
    }
    memcpy(file->filebuffer + offset, buf, nbytes);
    ssize_t end = (ssize_t)offset + (ssize_t)nbytes;
    if (end > file->len) {
        file->len = end;
    }
    return (ssize_t)nbytes;
}

static int test_close(int fd) {

    struct fake_file_buffer *file = fake_file_exists("", fd);
    if (file == NULL) {
        errno = EBADF;
        return -1;
    }

    file->fd = -1;
    return 0;
}

static int test_fsync(int fd) {

    struct fake_file_buffer *file = fake_file_exists(NULL, fd);
    if (file == NULL) {
        errno = EBADF;
        return -1;
    }
    return 0;
}

struct syscalls test_syscalls = {.openat = test_openat,
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
