#ifndef KERNEL_CORE_CONFIG_HPP
#define KERNEL_CORE_CONFIG_HPP

#include <stdint.h>

namespace core {

struct Config {
    bool serial_enabled;
    bool vga_enabled;
    uint32_t serial_baud;
    int log_level;  // 0=err, 1=warn, 2=info, 3=debug
};

Config* config_get();
void config_init();

}

#endif
