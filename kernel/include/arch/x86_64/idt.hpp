#ifndef KERNEL_ARCH_X86_64_IDT_HPP
#define KERNEL_ARCH_X86_64_IDT_HPP

#include <stdint.h>

namespace arch
{

// Exception vectors (0-31)
constexpr uint32_t EXCEPTION_COUNT = 32;
constexpr uint32_t VECTOR_DIVIDE_ERROR = 0;
constexpr uint32_t VECTOR_DEBUG = 1;
constexpr uint32_t VECTOR_BREAKPOINT = 3;
constexpr uint32_t VECTOR_INVALID_OPCODE = 6;
constexpr uint32_t VECTOR_DEVICE_NOT_AVAILABLE = 7;
constexpr uint32_t VECTOR_SEGMENT_NOT_PRESENT = 11;
constexpr uint32_t VECTOR_STACK_FAULT = 12;
constexpr uint32_t VECTOR_PAGE_FAULT = 14;
constexpr uint32_t VECTOR_X87_ERROR = 16;
constexpr uint32_t VECTOR_ALIGNMENT_CHECK = 17;
constexpr uint32_t VECTOR_SIMD_ERROR = 19;
/** Length of `int 0x80` (CD 80): a restarted syscall rewinds EIP by it. */
constexpr uint32_t SYSCALL_INSN_LEN = 2;
constexpr int IRQ_BASE = 32; // IRQ 0-15 map to 32-47
constexpr uint32_t IRQ_COUNT = 16;
constexpr uint32_t SYSCALL_VECTOR = 0x80;

// Page fault error code bits (Intel SDM 4.7.3).
constexpr uint32_t PF_PRESENT = 1u << 0; /* page was present (protection) */
constexpr uint32_t PF_WRITE = 1u << 1; /* fault was a write access */
constexpr uint32_t PF_USER = 1u << 2; /* fault happened at CPL 3 */

// Indices into the saved frame that isr_common passes as `regs`, counting
// dwords from the start of the pusha block. Above the pusha block the stub's
// vector and error code follow, then what the CPU pushed on entry; the four
// segment registers isr_common pushed sit *below* the block and are not part
// of it.
constexpr int FRAME_EDI = 0;
constexpr int FRAME_ESI = 1;
constexpr int FRAME_EBP = 2;
/** ESP of the interrupted context (always present). */
constexpr int FRAME_ESP = 3;
constexpr int FRAME_EBX = 4;
constexpr int FRAME_EDX = 5;
constexpr int FRAME_ECX = 6;
constexpr int FRAME_EAX = 7;
constexpr int FRAME_VECTOR = 8;
constexpr int FRAME_ERROR = 9;
constexpr int FRAME_EIP = 10;
constexpr int FRAME_CS = 11;
constexpr int FRAME_EFLAGS = 12;
/** ESP and SS are pushed by the CPU only when the privilege level changed, so
 *  these two are only valid when FRAME_CS is a user selector. */
constexpr int FRAME_STACK_ESP = 13;
constexpr int FRAME_STACK_SS = 14;

void idt_init();
void idt_load();

} // namespace arch

#endif
