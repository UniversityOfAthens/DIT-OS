// DIT-OS Kernel - Configuration

#include "core/config.hpp"

namespace core
{

static Config s_config = { true, true, 115200, 2 };

Config* config_get()
{
    return &s_config;
}

void config_init()
{
    (void)s_config;
}

} // namespace core
