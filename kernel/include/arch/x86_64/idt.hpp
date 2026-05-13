#ifndef KERNEL_ARCH_X86_64_IDT_HPP
#define KERNEL_ARCH_X86_64_IDT_HPP

#include <stdint.h>

namespace arch
{

// Exception vectors (0-31)
constexpr int IRQ_BASE = 32; // IRQ 0-15 map to 32-47

void idt_init();
void idt_load();

} // namespace arch

#endif
