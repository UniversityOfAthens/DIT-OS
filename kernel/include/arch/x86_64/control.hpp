#ifndef KERNEL_ARCH_X86_64_CONTROL_HPP
#define KERNEL_ARCH_X86_64_CONTROL_HPP

#include <stdint.h>

namespace arch
{

uint32_t read_cr0();
void write_cr0(uint32_t value);
/** Faulting linear address of the last page fault. */
uint32_t read_cr2();
uint32_t read_cr3();
void write_cr3(uint32_t value);
uint32_t read_cr4();
void write_cr4(uint32_t value);

constexpr uint32_t CR0_PE = 1u << 0;
constexpr uint32_t CR0_PG = 1u << 31;
constexpr uint32_t CR4_PAE = 1u << 5;
constexpr uint32_t CR4_PSE = 1u << 4;

} // namespace arch

#endif
