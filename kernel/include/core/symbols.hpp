#ifndef KERNEL_CORE_SYMBOLS_HPP
#define KERNEL_CORE_SYMBOLS_HPP

#include <stdint.h>

namespace core {

void symbols_init();
const char* symbol_lookup(uint32_t addr, uint32_t* out_offset);

}

#endif
