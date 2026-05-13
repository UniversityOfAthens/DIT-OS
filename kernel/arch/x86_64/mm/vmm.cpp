// =============================================================================
// DIT-OS Kernel - Virtual Memory Manager
// Uses arch paging; identity map or simple direct map. Full PD/PT later.
// =============================================================================

#include "mm/vmm.hpp"
#include "mm/page.hpp"
#include "arch/x86_64/paging.hpp"
#include <stdint.h>
#include <stddef.h>

namespace mm
{

static bool vmm_initialized;

void vmm_init()
{
    vmm_initialized = true;
}

int vmm_map(uint32_t vaddr, uint32_t paddr, size_t num_pages, uint32_t flags)
{
    (void)vaddr;
    (void)paddr;
    (void)num_pages;
    (void)flags;
    if (!vmm_initialized)
        return -1;
    return 0;
}

void vmm_unmap(uint32_t vaddr, size_t num_pages)
{
    (void)vaddr;
    (void)num_pages;
}

uint32_t vmm_virt_to_phys(uint32_t vaddr)
{
    (void)vaddr;
    return 0;
}

} // namespace mm
