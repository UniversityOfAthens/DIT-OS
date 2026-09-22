#ifndef KERNEL_ARCH_X86_64_IDT_HPP
#define KERNEL_ARCH_X86_64_IDT_HPP

#include <stdint.h>

namespace arch
{

// Exception vectors (0-31)
constexpr uint32_t EXCEPTION_COUNT = 32;
constexpr uint32_t VECTOR_PAGE_FAULT = 14;
constexpr int IRQ_BASE = 32; // IRQ 0-15 map to 32-47
constexpr uint32_t IRQ_COUNT = 16;
constexpr uint32_t SYSCALL_VECTOR = 0x80;

// Indices into the saved frame that isr_common passes as `regs`: the pusha
// block (EDI..EAX), then the stub's vector and error code, then what the CPU
// pushed on entry.
constexpr int FRAME_EAX = 7;
constexpr int FRAME_EIP = 10;
constexpr int FRAME_CS = 11;
constexpr int FRAME_EFLAGS = 12;

void idt_init();
void idt_load();

} // namespace arch

#endif
