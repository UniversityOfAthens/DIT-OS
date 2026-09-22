// =============================================================================
// DIT-OS Kernel - 8259 PIC init and mask
// =============================================================================

#include "arch/x86_64/pic.hpp"
#include "arch/x86_64/io.hpp"
#include "arch/x86_64/idt.hpp"
#include <stdint.h>

extern "C" void pic_send_eoi(uint8_t irq);

namespace arch
{

void pic_init()
{
    const uint8_t icw1 = 0x11;
    const uint8_t icw4 = 0x01;
    outb(PIC1_CMD, icw1);
    outb(PIC2_CMD, icw1);
    outb(PIC1_DATA, IRQ_BASE);
    outb(PIC2_DATA, IRQ_BASE + 8);
    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);
    outb(PIC1_DATA, icw4);
    outb(PIC2_DATA, icw4);
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void pic_mask(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t mask = inb(port) | (1u << (irq & 7));
    outb(port, mask);
}

void pic_unmask(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    uint8_t mask = inb(port) & ~(1u << (irq & 7));
    outb(port, mask);
}

void pic_eoi(uint8_t irq)
{
    pic_send_eoi(irq);
}

bool pic_is_spurious(uint8_t irq)
{
    if (irq != PIC_SPURIOUS_MASTER && irq != PIC_SPURIOUS_SLAVE)
        return false;
    // A real IRQ 7/15 has its bit set in the in-service register; a spurious
    // one (line dropped before the CPU acknowledged it) does not.
    uint16_t cmd = irq == PIC_SPURIOUS_MASTER ? PIC1_CMD : PIC2_CMD;
    outb(cmd, PIC_READ_ISR);
    if (inb(cmd) & (1u << (irq & 7)))
        return false;
    // The master did see a real IRQ 2 from the slave, so it still needs one.
    if (irq == PIC_SPURIOUS_SLAVE)
        outb(PIC1_CMD, PIC_EOI);
    return true;
}

} // namespace arch
