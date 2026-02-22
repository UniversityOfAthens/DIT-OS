// =============================================================================
// DIT-OS Kernel - Kernel heap allocator
// Simple bump allocator; will use PMM/VMM when paging is fully wired.
// =============================================================================

#include "mm/page.hpp"
#include <stdint.h>
#include <stddef.h>

namespace mm {

static uint8_t heap_buf[64 * 1024] __attribute__((aligned(8)));
static size_t heap_used;

void heap_init() {
    heap_used = 0;
}

void *heap_alloc(size_t size) {
    size = (size + 7u) & ~7u;
    if (heap_used + size > sizeof(heap_buf)) return nullptr;
    void *p = &heap_buf[heap_used];
    heap_used += size;
    return p;
}

void heap_free(void *) {
    // Bump allocator: no per-block free
}

} // namespace mm
