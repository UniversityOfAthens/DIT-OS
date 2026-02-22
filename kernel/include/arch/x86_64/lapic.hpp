#ifndef KERNEL_ARCH_X86_64_LAPIC_HPP
#define KERNEL_ARCH_X86_64_LAPIC_HPP

#include <stdint.h>

namespace arch {

uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t value);
void lapic_init();
void lapic_eoi();

} // namespace arch

#endif
