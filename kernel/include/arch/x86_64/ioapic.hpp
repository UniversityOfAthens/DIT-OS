#ifndef KERNEL_ARCH_X86_64_IOAPIC_HPP
#define KERNEL_ARCH_X86_64_IOAPIC_HPP

#include <stdint.h>

namespace arch {

void ioapic_init();
void ioapic_set_irq(uint8_t gsi, uint32_t vector, uint32_t flags);

} // namespace arch

#endif
