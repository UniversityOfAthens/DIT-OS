// =============================================================================
// DIT-OS Kernel - CPU features (uses cpuid)
// =============================================================================

#include "arch/x86_64/cpuid.hpp"

namespace arch
{

bool has_apic()
{
    return cpuid_has_feature_apic();
}
bool has_pae()
{
    return cpuid_has_feature_pae();
}

} // namespace arch
