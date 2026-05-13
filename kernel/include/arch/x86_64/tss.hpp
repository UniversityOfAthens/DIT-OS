#ifndef KERNEL_ARCH_X86_64_TSS_HPP
#define KERNEL_ARCH_X86_64_TSS_HPP

#include <stdint.h>

namespace arch
{

struct Tss {
    uint32_t reserved0;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldtr;
    uint16_t reserved1;
    uint16_t iomap_base;
} __attribute__((packed));

void tss_init();
void tss_set_kernel_stack(uint32_t esp0);

} // namespace arch

#endif
