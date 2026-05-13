#ifndef BOOTLOADER_MEMORY_HPP
#define BOOTLOADER_MEMORY_HPP

#include "types.hpp"

namespace boot
{

// E820 memory map entry type
enum class E820Type : u32 {
    Usable = 1,
    Reserved = 2,
    ACPIReclaim = 3,
    ACPINVS = 4,
    Bad = 5,
};

struct E820Entry {
    u64 base;
    u64 length;
    E820Type type;
    u32 acpi_attrs;
};

// Call from real mode; fills entries, returns count (max 32)
int memory_map_e820(E820Entry* entries, int max_count);

// Total usable memory (sum of type Usable) in bytes
u64 memory_usable_total(const E820Entry* entries, int count);

} // namespace boot

#endif
