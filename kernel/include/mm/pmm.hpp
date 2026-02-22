#ifndef KERNEL_MM_PMM_HPP
#define KERNEL_MM_PMM_HPP

#include <stdint.h>
#include <stddef.h>

namespace mm {

// Physical frame (page) allocator
// All addresses are physical.

void pmm_init();
void *pmm_alloc(size_t num_pages);
void pmm_free(void *physical, size_t num_pages);

// Optional: get total / free frame count (stub may return 0)
size_t pmm_total_frames();
size_t pmm_free_frames();

} // namespace mm

#endif
