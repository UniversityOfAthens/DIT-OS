#ifndef KERNEL_ARCH_X86_64_PIC_HPP
#define KERNEL_ARCH_X86_64_PIC_HPP

#include <stdint.h>

namespace arch
{

// 8259 PIC ports
constexpr uint16_t PIC1_CMD = 0x20;
constexpr uint16_t PIC1_DATA = 0x21;
constexpr uint16_t PIC2_CMD = 0xA0;
constexpr uint16_t PIC2_DATA = 0xA1;

void pic_init(); // Remap to IRQ_BASE (32), mask all
void pic_mask(uint8_t irq); // Mask one IRQ (0-15)
void pic_unmask(uint8_t irq); // Unmask one IRQ
void pic_eoi(uint8_t irq); // End of interrupt (0-15)

} // namespace arch

#endif
