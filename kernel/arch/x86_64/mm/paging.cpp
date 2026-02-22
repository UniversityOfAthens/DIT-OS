// DIT-OS Kernel - Page table management (32-bit)

#include "arch/x86_64/paging.hpp"
#include "arch/x86_64/control.hpp"
#include <stdint.h>

extern "C" void paging_load_impl(uint32_t pd_physical);
extern "C" void paging_enable_impl(void);
extern "C" void paging_disable_impl(void);
extern "C" void tlb_invalidate_impl(uint32_t vaddr);
extern "C" void tlb_flush_impl(void);

namespace arch {

void paging_load(uint32_t pd_physical) {
    paging_load_impl(pd_physical);
}

void tlb_invalidate(uint32_t vaddr) {
    tlb_invalidate_impl(vaddr);
}

void tlb_flush() {
    tlb_flush_impl();
}

void paging_enable() {
    paging_enable_impl();
}

void paging_disable() {
    paging_disable_impl();
}

} // namespace arch
