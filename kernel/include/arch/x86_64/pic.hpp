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

constexpr uint8_t PIC_EOI = 0x20; // OCW2: non-specific end of interrupt
constexpr uint8_t PIC_READ_ISR = 0x0B; // OCW3: next command-port read = ISR
constexpr uint8_t PIC_SPURIOUS_MASTER = 7;
constexpr uint8_t PIC_SPURIOUS_SLAVE = 15;

void pic_init(); // Remap to IRQ_BASE (32), mask all
void pic_mask(uint8_t irq); // Mask one IRQ (0-15)
void pic_unmask(uint8_t irq); // Unmask one IRQ
void pic_eoi(uint8_t irq); // End of interrupt (0-15)
/** True if IRQ 7/15 was spurious (no EOI must be sent for it); sends the
 * master EOI a spurious IRQ 15 still needs. */
bool pic_is_spurious(uint8_t irq);

} // namespace arch

#endif
