// DIT-OS Kernel - Panic handler

#include "core/panic.hpp"
#include "core/printk.hpp"
#include "dev/serial.hpp"
#include <stdarg.h>
#include <stdint.h>

namespace core {

void panic(const char* msg) {
    dev::serial_puts("\n*** KERNEL PANIC ***\n");
    dev::serial_puts(msg);
    dev::serial_puts("\n*** HALTED ***\n");
    for (;;) __asm__ volatile("cli; hlt");
}

void panicf(const char* fmt, ...) {
    dev::serial_puts("\n*** KERNEL PANIC ***\n");
    va_list args;
    va_start(args, fmt);
    core::vprintk(fmt, args);
    va_end(args);
    dev::serial_puts("\n*** HALTED ***\n");
    for (;;) __asm__ volatile("cli; hlt");
}

}
