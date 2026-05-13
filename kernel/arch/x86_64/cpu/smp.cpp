// =============================================================================
// DIT-OS Kernel - SMP (stub)
// =============================================================================

#include "arch/x86_64/smp.hpp"

namespace arch
{

int smp_get_cpu_count()
{
    return 1;
}

void smp_init()
{
}

} // namespace arch
