#ifndef KERNEL_ARCH_X86_64_APIC_HPP
#define KERNEL_ARCH_X86_64_APIC_HPP

#include <stdint.h>

namespace arch {

// Stub: APIC detection/init (use PIC for now)
bool apic_available();
void apic_init();

} // namespace arch

#endif
