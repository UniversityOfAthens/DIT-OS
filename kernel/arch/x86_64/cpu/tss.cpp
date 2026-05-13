// =============================================================================
// DIT-OS Kernel - TSS init and kernel stack
// =============================================================================

#include "arch/x86_64/tss.hpp"
#include <stdint.h>

extern "C" void tss_load(void);
extern "C" arch::Tss tss;

namespace arch
{

void tss_init()
{
    tss.ss0 = 0x10;
    tss.iomap_base = sizeof(Tss);
    tss_load();
}

void tss_set_kernel_stack(uint32_t esp0)
{
    tss.esp0 = esp0;
}

} // namespace arch
