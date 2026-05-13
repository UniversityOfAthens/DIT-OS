#ifndef KERNEL_ARCH_X86_64_ARCH_HPP
#define KERNEL_ARCH_X86_64_ARCH_HPP

namespace arch
{

void cpu_early_init(); // GDT, IDT, TSS, PIC
void cpu_init(); // Full CPU init (called from kernel_main)

} // namespace arch

#endif
