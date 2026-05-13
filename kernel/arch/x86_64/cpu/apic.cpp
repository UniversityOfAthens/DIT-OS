// =============================================================================
// DIT-OS Kernel - APIC (stub; use PIC for now)
// =============================================================================

#include "arch/x86_64/apic.hpp"

namespace arch
{

bool apic_available()
{
    return false;
}

void apic_init()
{
}

} // namespace arch
