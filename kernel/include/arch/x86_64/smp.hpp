#ifndef KERNEL_ARCH_X86_64_SMP_HPP
#define KERNEL_ARCH_X86_64_SMP_HPP

#include <stdint.h>

namespace arch
{

int smp_get_cpu_count();
void smp_init();

} // namespace arch

#endif
