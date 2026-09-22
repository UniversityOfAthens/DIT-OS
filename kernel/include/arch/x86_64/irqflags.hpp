#ifndef KERNEL_ARCH_X86_64_IRQFLAGS_HPP
#define KERNEL_ARCH_X86_64_IRQFLAGS_HPP

// DIT-OS Kernel - Save/restore the CPU interrupt flag

#include <stdint.h>

namespace arch
{

constexpr uint32_t EFLAGS_IF = 1u << 9; // interrupts enabled
constexpr uint32_t EFLAGS_RESERVED = 1u << 1; // always reads as 1

/** Disable interrupts and return the previous EFLAGS for irq_restore(). */
inline uint32_t irq_save()
{
    uint32_t flags;
    __asm__ volatile("pushfl; popl %0; cli" : "=r"(flags) : : "memory");
    return flags;
}

/** Re-enable interrupts if they were enabled when irq_save() was called. */
inline void irq_restore(uint32_t flags)
{
    if (flags & EFLAGS_IF)
        __asm__ volatile("sti" : : : "memory");
}

} // namespace arch

#endif // KERNEL_ARCH_X86_64_IRQFLAGS_HPP
