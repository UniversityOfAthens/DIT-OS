// =============================================================================
// DIT-OS Kernel - MSR C++ wrapper
// =============================================================================

#include "arch/x86_64/msr.hpp"

extern "C" uint64_t rdmsr_impl(uint32_t msr);
extern "C" void wrmsr_impl(uint32_t msr, uint64_t value);

namespace arch
{

uint64_t rdmsr(uint32_t msr)
{
    return rdmsr_impl(msr);
}

void wrmsr(uint32_t msr, uint64_t value)
{
    wrmsr_impl(msr, value);
}

} // namespace arch
