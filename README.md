# superlite3

A single-file embedded database written from scratch in C, modeled on SQLite3.

superlite3 is built bottom-up, one layer at a time, with each layer tested before the next one starts. It's a learning project with production habits: every syscall is wrapped, every failure path has a test, and the design choices are written down along with the reasons for them.

> **Status: early.** The OS layer is complete and tested. The pager is in design. Nothing above it exists yet, so you can't store data with it today. If you want to follow along, watch the repo.

---

## Why

Most "build your own database" projects stop at an in-memory B-tree. superlite3 is about the parts that make a database trustworthy: durable writes, crash recovery, and knowing exactly where every `fsync` goes and why.

## Design

superlite3 follows SQLite's layering. Each layer talks only to the one below it.

```
┌────────────────────────────┐
│  SQL front end             │  planned
├────────────────────────────┤
│  B-tree                    │  planned: tables and indexes
├────────────────────────────┤
│  Pager                     │  in design: page cache, WAL
├────────────────────────────┤
│  OS layer                  │  done: syscall wrappers
└────────────────────────────┘
          one database file + one WAL file
```

### Decisions so far

| Decision | Choice | Reason |
|---|---|---|
| Storage | One database file | Gives one `fsync` target, and copying the database is copying a file. All tables and indexes are B-trees in the same file, found through their root page numbers. |
| Page size | 4096 bytes | Matches the file system block and device block on current hardware, so one page write is one block write. |
| Journaling | WAL only, no rollback journal | Commit is one `fsync` on the WAL. The database file is written only at checkpoints. |
| Page cache | Managed by the pager, not `mmap` | The pager decides when dirty pages reach disk, and I/O errors come back as error codes instead of `SIGBUS`. |
| Eviction | LRU, skipping pinned pages | A page the B-tree is holding is never evicted. |
| I/O | Positional (`pread`/`pwrite`) | One fd per file, and every access names its own offset. |

### The OS layer

`src/os/` wraps every syscall the database uses: `openat`, `stat`, `pread`, `pwrite`, `fsync` and `close`. The wrappers:

- retry on `EINTR`
- loop on short reads and writes until all the bytes are transferred
- treat an unexpected EOF as an error rather than a short result
- return `0` or `-1`, with `errno` left as the syscall set it

The syscalls are called through a function-pointer table (`struct syscalls`), which tests replace with a fake backend. That makes it possible to test failures that are hard to trigger on a real disk, such as `EINTR` partway through a read, a short read at EOF, or permission errors.

---

## Building

Requires a C compiler and `make`. Developed and tested on macOS (arm64). It uses only POSIX calls, but Linux hasn't been tested yet.

```sh
make build    # builds ./superlite
make debug    # builds ./superlite_gdb with debug symbols
```

## Testing

```sh
make test
./test_superlite
```

The suite prints one line per assertion and exits non-zero if any assertion fails. It currently covers the OS layer with 51 assertions, including:

- create, stat and open, including `ENOENT`, `EACCES` and `EEXIST`
- round-trip read/write integrity, at offset 0 and at other offsets
- unexpected EOF on a short read
- recovery from `EINTR` on reads and writes, with a check that the retries actually happened
- zero-byte writes, `close` and `fsync`, including invalid file descriptors

---

## Roadmap

### ✅ M0: OS layer
Syscall wrappers with `EINTR` and short-I/O handling, an injectable syscall backend, and a fake backend with failure injection. Full test coverage of every wrapper.

### 🔨 M1: Pager, read path
- [ ] Open a database file and read page size from the header
- [ ] Fixed-size page cache on the heap
- [ ] `get page` / `release page` with reference counts (pinning)
- [ ] LRU eviction that skips pinned pages
- [ ] Tests: cache hits, misses, eviction order, all-pinned behavior

### M2: Pager, write path and WAL
- [ ] Mark a page as writable and track dirty pages
- [ ] Allocate pages from the freelist or the end of the file
- [ ] WAL file format: header, frames, commit frames
- [ ] Commit: append frames, then `fsync` the WAL
- [ ] WAL index: page number → latest frame
- [ ] Read path checks the WAL before the database file
- [ ] Spilling dirty pages to the WAL under cache pressure
- [ ] Checkpoint: copy frames to the database file, `fsync`, reset the WAL
- [ ] Crash recovery: replay frames up to the last commit frame

### M3: File format
- [ ] 100-byte database header on page 1
- [ ] Schema table on page 1
- [ ] Freelist

### M4: B-tree, tables
- [ ] Slotted-page layout for cells
- [ ] Table B-trees keyed by rowid
- [ ] Insert, lookup, delete and page splits
- [ ] Overflow pages for large rows
- [ ] Cursors for ordered scans

### M5: B-tree, indexes
- [ ] Index B-trees
- [ ] Merging pages after deletes

### M6: Query layer
- [ ] A small SQL subset: `CREATE TABLE`, `INSERT`, `SELECT ... WHERE`, `DELETE`
- [ ] Tokenizer and parser
- [ ] Execution against the B-tree layer

### Later
- File locking, so more than one process can safely open a database
- Durability on macOS with `F_FULLFSYNC`
- Testing on Linux
- Fault-injection tests for crashes during commit and checkpoint

## Non-goals

- Compatibility with SQLite's file format
- Running as a server or handling network access
- Full SQL

---

## Repository layout

```
src/
  main.c              entry point
  os/
    os.h, os.c        syscall wrappers
    backend.h         struct syscalls and swap_backend()
    real_backend.c    the backend that calls the real syscalls
tests/
  test_main.c         test driver
  os/
    test_backend.c    fake backend: an in-memory file table with failure injection
    test_backend.h
```

## Contributing

Issues and discussion are welcome, especially:

- design critiques with a reason attached ("this breaks if a crash happens between X and Y")
- test cases for failure modes that aren't covered yet
- reports from running the test suite on Linux

The project is written by hand as a learning exercise, so pull requests that implement roadmap items may not be merged. Opening an issue first to talk about the design is the most useful way to help.

## References

These are the sources the design is based on.

- [SQLite: Database File Format](https://www.sqlite.org/fileformat.html)
- [SQLite: Atomic Commit](https://www.sqlite.org/atomiccommit.html)
- [SQLite: Write-Ahead Logging](https://www.sqlite.org/wal.html)
- [SQLite: Architecture](https://www.sqlite.org/arch.html)
- Alex Petrov, *Database Internals* (O'Reilly, 2019)
- Remzi and Andrea Arpaci-Dusseau, [*Operating Systems: Three Easy Pieces*](https://pages.cs.wisc.edu/~remzi/OSTEP/)
- Crotty, Leis and Pavlo, [*Are You Sure You Want to Use MMAP in Your Database Management System?*](https://www.cidrdb.org/cidr2022/papers/p13-crotty.pdf) (CIDR 2022)
- CMU 15-445/645, Database Systems (Andy Pavlo)

## License

TODO: add a license.
