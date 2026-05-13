// Minimal userland malloc: bump allocator

#include "stdlib.h"
#include <stddef.h>

static unsigned char s_heap[64 * 1024];
static size_t s_used = 0;

static inline size_t align_up(size_t n)
{
    return (n + 7u) & ~7u;
}

void* malloc(size_t size)
{
    size = align_up(size);
    if (s_used + size > sizeof(s_heap))
        return NULL;
    void* p = &s_heap[s_used];
    s_used += size;
    return p;
}

void free(void* ptr)
{
    (void)ptr;
    // Bump allocator: no-op free.
}
