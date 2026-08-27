#include "../src/os/backend.h"
#include "../src/os/os.h"

#include <assert.h>
#include <errno.h>

extern struct syscalls test_syscalls;

int file_not_exists() {
    struct syscalls *prev = swap_backend(&test_syscalls);

    const char *fake_path = "/fake/path.c";
    struct stat *statbuf = {};

    int out = stat_file(fake_path, statbuf);
    assert(out == ENOENT);
    assert(errno == ENOENT);

    swap_backend(prev);

    return 0;
}

int main() { file_not_exists(); }

/* file existence and creation:
* 1. File doesn't exist → stat_file reports it doesn't exist (ENOENT)
2. File exists → stat_file confirms it exists
3. File doesn't exist → create_file succeeds
4. File doesn't exist, permission denied on the containing dir → create_file
fails, EACCES
5. [new] File already exists → create_file fails, EEXIST — this is the entire
reason O_EXCL is set; currently untested
 * */
