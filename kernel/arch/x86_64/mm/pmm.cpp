// =============================================================================
// DIT-OS Kernel - Physical Memory Manager
// Simple bitmap allocator; memory map from multiboot/e820 to be wired later.
// =============================================================================

#include "mm/pmm.hpp"
#include "mm/page.hpp"
#include <stdint.h>
#include <stddef.h>

namespace mm {

static constexpr size_t MAX_FRAMES = 32768;  // 128 MB / 4 KB
static uint32_t frame_bitmap[(MAX_FRAMES + 31) / 32];
static size_t total_frames_set;
static bool initialized;

void pmm_init() {
    for (size_t i = 0; i < sizeof(frame_bitmap) / sizeof(frame_bitmap[0]); i++) {
        frame_bitmap[i] = 0;
    }
    total_frames_set = 0;
    initialized = true;
}

void *pmm_alloc(size_t num_pages) {
    if (!initialized || num_pages == 0) return nullptr;
    for (size_t start = 0; start + num_pages <= MAX_FRAMES; start++) {
        size_t i;
        for (i = 0; i < num_pages; i++) {
            size_t idx = (start + i) >> 5;
            uint32_t bit = 1u << ((start + i) & 31u);
            if (frame_bitmap[idx] & bit) break;
        }
        if (i == num_pages) {
            for (i = 0; i < num_pages; i++) {
                size_t idx = (start + i) >> 5;
                uint32_t bit = 1u << ((start + i) & 31u);
                frame_bitmap[idx] |= bit;
            }
            total_frames_set += num_pages;
            return reinterpret_cast<void *>(start * PAGE_SIZE);
        }
    }
    return nullptr;
}

void pmm_free(void *physical, size_t num_pages) {
    if (!physical || num_pages == 0) return;
    uintptr_t base = reinterpret_cast<uintptr_t>(physical);
    size_t start = base / PAGE_SIZE;
    for (size_t i = 0; i < num_pages; i++) {
        size_t idx = (start + i) >> 5;
        uint32_t bit = 1u << ((start + i) & 31u);
        frame_bitmap[idx] &= ~bit;
    }
    if (total_frames_set >= num_pages) {
        total_frames_set -= num_pages;
    }
}

size_t pmm_total_frames() {
    return MAX_FRAMES;
}

size_t pmm_free_frames() {
    return initialized ? (MAX_FRAMES - total_frames_set) : 0;
}

} // namespace mm
