#ifndef KERNEL_ARCH_X86_64_MSR_HPP
#define KERNEL_ARCH_X86_64_MSR_HPP

#include <stdint.h>

namespace arch
{

uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);

// Common MSRs
constexpr uint32_t MSR_EFER = 0xC0000080;
constexpr uint32_t MSR_STAR = 0xC0000081;
constexpr uint32_t MSR_LSTAR = 0xC0000082;
constexpr uint32_t MSR_FMASK = 0xC0000084;
constexpr uint32_t MSR_APIC_BASE = 0x1B;

} // namespace arch

#endif
