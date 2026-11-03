// =============================================================================
// DIT-OS Kernel - IDT setup and interrupt dispatch
// =============================================================================

#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/pic.hpp"
#include "sched/signal.hpp"
#include <stdint.h>

extern "C" void load_idt(void* descriptor);
extern "C" uint32_t idt_stub_addrs[256];
extern "C" void syscall_entry(uint32_t* regs);

namespace arch
{

static uint8_t idt_entries[256 * 8] __attribute__((aligned(8)));
static uint8_t idt_desc[6] __attribute__((aligned(2)));

void idt_load()
{
    uint16_t limit = sizeof(idt_entries) - 1;
    uint32_t base = reinterpret_cast<uint32_t>(idt_entries);
    idt_desc[0] = limit & 0xFF;
    idt_desc[1] = (limit >> 8) & 0xFF;
    idt_desc[2] = base & 0xFF;
    idt_desc[3] = (base >> 8) & 0xFF;
    idt_desc[4] = (base >> 16) & 0xFF;
    idt_desc[5] = (base >> 24) & 0xFF;
    load_idt(idt_desc);
}

void idt_init()
{
    for (int i = 0; i < 256; i++)
    {
        uint32_t offset = idt_stub_addrs[i];
        uint8_t* e = idt_entries + i * 8;
        e[0] = offset & 0xFF;
        e[1] = (offset >> 8) & 0xFF;
        e[2] = 0x08; // selector low
        e[3] = 0x00; // selector high
        e[4] = 0;
        e[5] = (static_cast<uint32_t>(i) == SYSCALL_VECTOR) ?
                   0xEE :
                   0x8E; // 0x80: DPL=3 so user can int 0x80; else ring 0
        e[6] = (offset >> 16) & 0xFF;
        e[7] = (offset >> 24) & 0xFF;
    }
    idt_load();
}

extern "C" void interrupt_handler(uint32_t vector, uint32_t error_code,
                                  uint32_t* regs);

// Called from asm for every interrupt (vector 0-255, error_code for exceptions)
// regs points to the saved frame (see FRAME_* in idt.hpp)
extern "C" void interrupt_dispatch(uint32_t vector, uint32_t error_code,
                                   uint32_t* regs)
{
    if (vector == SYSCALL_VECTOR)
    {
        syscall_entry(regs);
        return;
    }
    if (vector >= IRQ_BASE && vector < IRQ_BASE + IRQ_COUNT)
    {
        uint8_t irq = static_cast<uint8_t>(vector - IRQ_BASE);
        if (pic_is_spurious(irq))
            return;
        pic_eoi(irq);
    }
    interrupt_handler(vector, error_code, regs);
    /* There is no preemption, so returning to ring 3 from an IRQ or fault is
     * the only chance to act on a signal raised meanwhile (^C on a task
     * spinning in user mode, or the fault's own signal). */
    if ((regs[FRAME_CS] & 3u) != 0)
        (void)sched::signal_handle_pending(regs);
}

} // namespace arch
