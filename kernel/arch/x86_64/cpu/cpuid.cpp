// =============================================================================
// DIT-OS Kernel - CPUID feature detection
// =============================================================================

#include "arch/x86_64/cpuid.hpp"
#include <stdint.h>

extern "C" void cpuid_impl(uint32_t eax_in, uint32_t ecx_in,
    uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);

namespace arch {

CpuidResult cpuid(uint32_t eax_in, uint32_t ecx_in) {
    CpuidResult r = {0, 0, 0, 0};
    cpuid_impl(eax_in, ecx_in, &r.eax, &r.ebx, &r.ecx, &r.edx);
    return r;
}

bool cpuid_has_feature_apic() {
    CpuidResult r = cpuid(1, 0);
    return (r.edx & (1u << 9)) != 0;
}

bool cpuid_has_feature_pae() {
    CpuidResult r = cpuid(1, 0);
    return (r.edx & (1u << 6)) != 0;
}

const char* cpuid_vendor_string(char buf[13]) {
    CpuidResult r = cpuid(0, 0);
    *(uint32_t*)(buf + 0) = r.ebx;
    *(uint32_t*)(buf + 4) = r.edx;
    *(uint32_t*)(buf + 8) = r.ecx;
    buf[12] = '\0';
    return buf;
}

} // namespace arch
