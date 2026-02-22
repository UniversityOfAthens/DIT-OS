// DIT-OS Kernel - Symbol table (stub)

#include "core/symbols.hpp"

namespace core {

void symbols_init() {
}

const char* symbol_lookup(uint32_t addr, uint32_t* out_offset) {
    (void)addr;
    if (out_offset) *out_offset = 0;
    return nullptr;
}

}
