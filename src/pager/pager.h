#ifndef BACKEND_DECLARATION
#define BACKEND_DECLARATION

#include <sys/stat.h>
#include <sys/types.h>

void *pgr_open(int pgno);
int *pgr_get(int pgno);
void pgr_release(int pgno);
void pgr_mark_dirty(int pgno);
void pgr_commit(int pgno); // partial

int *pgr_close(int pgno);

#endif
