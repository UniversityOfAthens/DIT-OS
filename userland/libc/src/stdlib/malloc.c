// DIT-OS userland - first-fit free-list over a static arena
//
// Mirrors the kernel heap: every block starts with a header, blocks are laid
// out back to back and adjacent free blocks are merged while searching. The
// arena is a fixed 64 KiB static buffer; there is no brk() growth yet, so a
// process that holds more than that at once still fails, but freed memory is
// reused instead of leaked.
//
// Single-threaded by construction (userland has no threads). Misuse that C
// leaves undefined - freeing a foreign or already freed pointer, or a
// corrupted block header - ends the process instead of letting it continue
// with a damaged arena.

#include "stdlib.h"
#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE (64u * 1024u)
#define HEAP_ALIGN 8u
#define BLOCK_MAGIC_FREE 0xF4EEB10Cu
#define BLOCK_MAGIC_USED 0xA110CA7Eu

typedef struct BlockHeader
{
    size_t size; /* whole block including this header */
    uint32_t magic; /* BLOCK_MAGIC_FREE or BLOCK_MAGIC_USED */
} BlockHeader;

/* Smallest block worth splitting off: a header plus one aligned unit. */
#define MIN_SPLIT (sizeof(BlockHeader) + HEAP_ALIGN)

static unsigned char s_heap[HEAP_SIZE] __attribute__((aligned(HEAP_ALIGN)));
static int s_initialised;

static BlockHeader* block_at(size_t offset)
{
    return (BlockHeader*)&s_heap[offset];
}

static BlockHeader* next_block(BlockHeader* b)
{
    /* A short or oversized block header would stop the walk making forward
     * progress or step outside the arena; treat it as corruption. */
    if (b->size < MIN_SPLIT || b->size > HEAP_SIZE)
        _exit(1);
    unsigned char* next = (unsigned char*)b + b->size;
    if (next > s_heap + HEAP_SIZE)
        _exit(1);
    if (next == s_heap + HEAP_SIZE)
        return NULL;
    return (BlockHeader*)next;
}

/* Absorb every free block that directly follows @b (which must be free). */
static void merge_following(BlockHeader* b)
{
    BlockHeader* n = next_block(b);
    while (n && n->magic == BLOCK_MAGIC_FREE)
    {
        b->size += n->size;
        n->magic = 0;
        n = next_block(b);
    }
}

void* malloc(size_t size)
{
    if (!s_initialised)
    {
        BlockHeader* first = block_at(0);
        first->size = HEAP_SIZE;
        first->magic = BLOCK_MAGIC_FREE;
        s_initialised = 1;
    }

    /* Bound before aligning: the add below would wrap for sizes near
     * SIZE_MAX (M38), and after this check the aligned size cannot. */
    if (size == 0 || size > HEAP_SIZE - sizeof(BlockHeader))
        return NULL;
    size_t need =
        sizeof(BlockHeader) + ((size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));

    for (BlockHeader* b = block_at(0); b; b = next_block(b))
    {
        if (b->magic == BLOCK_MAGIC_USED)
            continue;
        if (b->magic != BLOCK_MAGIC_FREE)
            _exit(1); /* corrupted arena: stop before following it */

        merge_following(b);
        if (b->size < need)
            continue;

        if (b->size - need >= MIN_SPLIT)
        {
            BlockHeader* rest = (BlockHeader*)((unsigned char*)b + need);
            rest->size = b->size - need;
            rest->magic = BLOCK_MAGIC_FREE;
            b->size = need;
        }
        b->magic = BLOCK_MAGIC_USED;
        return (void*)(b + 1);
    }
    return NULL;
}

void free(void* ptr)
{
    if (!ptr)
        return;

    unsigned char* p = (unsigned char*)ptr;
    if (p < s_heap + sizeof(BlockHeader) || p >= s_heap + HEAP_SIZE)
        _exit(1);
    BlockHeader* b = (BlockHeader*)p - 1;
    /* Freeing an already free block (double free) or a pointer that is not
     * one of our payloads would make the arena unrecoverable. */
    if (b->magic != BLOCK_MAGIC_USED)
        _exit(1);

    b->magic = BLOCK_MAGIC_FREE;
    merge_following(b);
}
