// =============================================================================
// DIT-OS Kernel - Kernel heap allocator
// First-fit free-list allocator over a static arena; will use PMM/VMM when
// paging is fully wired.
//
// Every block starts with a BlockHeader. Blocks are laid out back to back, so
// the next block is found by adding the size. Adjacent free blocks are merged
// when a block is freed and while searching for a fit.
//
// Not interrupt-safe: must only be called from task context (the scheduler is
// cooperative, so no locking is needed yet).
// =============================================================================

#include "mm/kmalloc.hpp"
#include "core/panic.hpp"
#include <stdint.h>
#include <stddef.h>

namespace mm
{

constexpr size_t HEAP_SIZE = 256 * 1024;
constexpr size_t HEAP_ALIGN = 8;
constexpr uint32_t BLOCK_MAGIC_FREE = 0xF4EEB10C;
constexpr uint32_t BLOCK_MAGIC_USED = 0xA110CA7E;

struct BlockHeader
{
    uint32_t size; /* whole block including this header */
    uint32_t magic; /* BLOCK_MAGIC_FREE or BLOCK_MAGIC_USED */
};

static_assert(sizeof(BlockHeader) % HEAP_ALIGN == 0,
              "header must keep payloads aligned");

/* Smallest block worth splitting off: a header plus one aligned unit. */
constexpr size_t MIN_SPLIT = sizeof(BlockHeader) + HEAP_ALIGN;

static uint8_t s_heap[HEAP_SIZE] __attribute__((aligned(HEAP_ALIGN)));

static BlockHeader* block_at(size_t offset)
{
    return reinterpret_cast<BlockHeader*>(&s_heap[offset]);
}

static BlockHeader* next_block(BlockHeader* b)
{
    uint8_t* next = reinterpret_cast<uint8_t*>(b) + b->size;
    if (next >= s_heap + HEAP_SIZE)
        return nullptr;
    return reinterpret_cast<BlockHeader*>(next);
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

void heap_init()
{
    BlockHeader* first = block_at(0);
    first->size = HEAP_SIZE;
    first->magic = BLOCK_MAGIC_FREE;
}

void* heap_alloc(size_t size)
{
    if (size == 0 || size > HEAP_SIZE - sizeof(BlockHeader))
        return nullptr;
    size_t need =
        sizeof(BlockHeader) + ((size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1));

    for (BlockHeader* b = block_at(0); b; b = next_block(b))
    {
        if (b->magic == BLOCK_MAGIC_USED)
            continue;
        if (b->magic != BLOCK_MAGIC_FREE)
            core::panic("heap_alloc: corrupted block header");

        merge_following(b);
        if (b->size < need)
            continue;

        if (b->size - need >= MIN_SPLIT)
        {
            BlockHeader* rest = reinterpret_cast<BlockHeader*>(
                reinterpret_cast<uint8_t*>(b) + need);
            rest->size = b->size - static_cast<uint32_t>(need);
            rest->magic = BLOCK_MAGIC_FREE;
            b->size = static_cast<uint32_t>(need);
        }
        b->magic = BLOCK_MAGIC_USED;
        return b + 1;
    }
    return nullptr;
}

void heap_free(void* ptr)
{
    if (!ptr)
        return;
    uint8_t* p = static_cast<uint8_t*>(ptr);
    if (p < s_heap + sizeof(BlockHeader) || p >= s_heap + HEAP_SIZE)
        core::panic("heap_free: pointer outside kernel heap");

    BlockHeader* b = reinterpret_cast<BlockHeader*>(p) - 1;
    if (b->magic == BLOCK_MAGIC_FREE)
        core::panic("heap_free: double free");
    if (b->magic != BLOCK_MAGIC_USED)
        core::panic("heap_free: invalid pointer or corrupted header");

    b->magic = BLOCK_MAGIC_FREE;
    merge_following(b);
}

} // namespace mm
