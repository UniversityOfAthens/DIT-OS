/** DIT-OS Kernel - Default build configuration (used when config.hpp not generated). */
#ifndef KERNEL_KCONFIG_DEFAULTS_HPP
#define KERNEL_KCONFIG_DEFAULTS_HPP

#ifndef CONFIG_SERIAL
#define CONFIG_SERIAL 1
#endif

#ifndef CONFIG_VGA
#define CONFIG_VGA 1
#endif

#ifndef CONFIG_DEBUG
#define CONFIG_DEBUG 0
#endif

#ifndef CONFIG_SMP
#define CONFIG_SMP 0
#endif

#ifndef CONFIG_SERIAL_BAUD
#define CONFIG_SERIAL_BAUD 115200
#endif

#endif
