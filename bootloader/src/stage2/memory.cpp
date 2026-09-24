// =============================================================================
// Memory map detection - E820 wrapper
// =============================================================================

#include "../../include/memory.hpp"
#include "../../include/types.hpp"

namespace boot
{

// E820 must be called from real mode (int 15h). When stage2 is built as 32-bit,
// use a realmode shim or call before switching to PM. Stub returns 0 here.
int memory_map_e820(E820Entry* entries, int max_count)
{
    (void)entries;
    (void)max_count;
    return 0;
}

u64 memory_usable_total(const E820Entry* entries, int count)
{
    u64 total = 0;
    for (int i = 0; i < count; i++)
    {
        if (entries[i].type == E820Type::Usable)
            total += entries[i].length;
    }
    return total;
}

} // namespace boot
