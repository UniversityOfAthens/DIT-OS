#ifndef KERNEL_ARCH_X86_64_CPUID_HPP
#define KERNEL_ARCH_X86_64_CPUID_HPP

#include <stdint.h>

namespace arch
{

struct CpuidResult {
    uint32_t eax, ebx, ecx, edx;
};

CpuidResult cpuid(uint32_t eax_in, uint32_t ecx_in = 0);

bool cpuid_has_feature_apic();
bool cpuid_has_feature_pae();
const char* cpuid_vendor_string(char buf[13]);

} // namespace arch

#endif
