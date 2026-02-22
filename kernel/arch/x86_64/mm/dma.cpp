// =============================================================================
// DIT-OS Kernel - Direct Memory Access (DMA) buffers
// Stub: allocate physically contiguous memory for device DMA.
// =============================================================================

#include "mm/pmm.hpp"
#include "mm/page.hpp"
#include <stdint.h>
#include <stddef.h>

namespace mm {

void *dma_alloc(size_t size) {
    size_t np = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    return pmm_alloc(np);
}

void dma_free(void *ptr, size_t size) {
    if (!ptr) return;
    size_t np = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    pmm_free(ptr, np);
}

} // namespace mm
