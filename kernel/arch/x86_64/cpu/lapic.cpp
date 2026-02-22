// =============================================================================
// DIT-OS Kernel - Local APIC (stub)
// =============================================================================

#include "arch/x86_64/lapic.hpp"

namespace arch {

uint32_t lapic_read(uint32_t) { return 0; }
void lapic_write(uint32_t, uint32_t) {}
void lapic_init() {}
void lapic_eoi() {}

} // namespace arch
