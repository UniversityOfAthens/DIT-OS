// DIT-OS Kernel - Core kernel entry / high-level init

#include "core/kernel.hpp"
#include "core/version.hpp"
#include "core/console.hpp"
#include "core/printk.hpp"
#include "core/config.hpp"
#include "core/cmdline.hpp"
#include "core/logo.hpp"
#include "core/panic.hpp"

namespace core {

void kernel_early_init() {
    config_init();
}

void kernel_main_continue() {
    console_init();
    logo_print();
    printk("Kernel core ready. Version: %s\n", version_string());
    if (cmdline_get()[0] != '\0')
        printk("Command line: %s\n", cmdline_get());
}

}
