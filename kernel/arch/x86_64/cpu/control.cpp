// =============================================================================
// DIT-OS Kernel - Control registers C++ wrapper
// =============================================================================

#include "arch/x86_64/control.hpp"

extern "C" uint32_t read_cr0_impl(void);
extern "C" void write_cr0_impl(uint32_t);
extern "C" uint32_t read_cr3_impl(void);
extern "C" void write_cr3_impl(uint32_t);
extern "C" uint32_t read_cr4_impl(void);
extern "C" void write_cr4_impl(uint32_t);

namespace arch {

uint32_t read_cr0() { return read_cr0_impl(); }
void write_cr0(uint32_t value) { write_cr0_impl(value); }
uint32_t read_cr3() { return read_cr3_impl(); }
void write_cr3(uint32_t value) { write_cr3_impl(value); }
uint32_t read_cr4() { return read_cr4_impl(); }
void write_cr4(uint32_t value) { write_cr4_impl(value); }

} // namespace arch
