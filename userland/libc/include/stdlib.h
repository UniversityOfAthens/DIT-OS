#ifndef USERLAND_LIBC_STDLIB_H
#define USERLAND_LIBC_STDLIB_H

#include <stddef.h>

void* malloc(size_t size);
void  free(void* ptr);

// _exit wrapper (may be used for libc-style termination)
void  _exit(int code);

#endif

