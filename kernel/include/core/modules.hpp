#ifndef KERNEL_CORE_MODULES_HPP
#define KERNEL_CORE_MODULES_HPP

#include <stdint.h>

namespace core {

struct Module {
    const char* name;
    uint32_t start;
    uint32_t end;
};

void modules_init();
unsigned int modules_count();
const Module* module_get(unsigned int i);

}

#endif
