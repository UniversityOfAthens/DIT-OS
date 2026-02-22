// DIT-OS Kernel - Module support (stub)

#include "core/modules.hpp"

namespace core {

static unsigned int s_count = 0;
static Module s_modules[1];

void modules_init() {
    s_count = 0;
}

unsigned int modules_count() {
    return s_count;
}

const Module* module_get(unsigned int i) {
    if (i >= s_count) return nullptr;
    return &s_modules[i];
}

}
