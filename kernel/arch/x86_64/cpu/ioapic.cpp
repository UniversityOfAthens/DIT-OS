// =============================================================================
// DIT-OS Kernel - I/O APIC (stub)
// =============================================================================

#include "arch/x86_64/ioapic.hpp"

namespace arch {

void ioapic_init() {}
void ioapic_set_irq(uint8_t, uint32_t, uint32_t) {}

} // namespace arch
