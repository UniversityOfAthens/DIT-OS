#ifndef KERNEL_MM_VMM_HPP
#define KERNEL_MM_VMM_HPP

#include <stdint.h>
#include <stddef.h>

namespace mm
{

// Virtual memory manager: map/unmap virtual to physical
// All vaddr are kernel virtual; paddr are physical.

void vmm_init();
int vmm_map(uint32_t vaddr, uint32_t paddr, size_t num_pages, uint32_t flags);
void vmm_unmap(uint32_t vaddr, size_t num_pages);
uint32_t vmm_virt_to_phys(uint32_t vaddr);

} // namespace mm

#endif
