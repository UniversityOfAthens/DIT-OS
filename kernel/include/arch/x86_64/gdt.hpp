#ifndef KERNEL_ARCH_X86_64_GDT_HPP
#define KERNEL_ARCH_X86_64_GDT_HPP

#include <stdint.h>

namespace arch {

// Segment selectors (must match gdt.asm order)
constexpr uint16_t GDT_SEL_NULL = 0x00;
constexpr uint16_t GDT_SEL_CODE = 0x08;
constexpr uint16_t GDT_SEL_DATA = 0x10;
constexpr uint16_t GDT_SEL_TSS  = 0x18;

void gdt_load();

} // namespace arch

#endif
