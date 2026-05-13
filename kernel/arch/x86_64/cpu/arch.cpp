// =============================================================================
// DIT-OS Kernel - CPU/arch early init and init
// =============================================================================

#include "arch/x86_64/arch.hpp"
#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/tss.hpp"
#include "arch/x86_64/pic.hpp"

namespace arch
{

void cpu_early_init()
{
    gdt_load();
    idt_init();
    tss_init();
    pic_init();
}

void cpu_init()
{
    cpu_early_init();
}

} // namespace arch
