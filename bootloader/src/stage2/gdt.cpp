// =============================================================================
// GDT setup - C++ helpers (actual table in gdt.asm)
// =============================================================================

#include "../include/types.hpp"

extern "C" void load_gdt(void);

namespace boot {

void gdt_load() {
    load_gdt();
}

} // namespace boot
