#ifndef KERNEL_CORE_PRINTK_HPP
#define KERNEL_CORE_PRINTK_HPP

#include <stdarg.h>

namespace core {

int printk(const char* fmt, ...);
int vprintk(const char* fmt, va_list args);
int printk_info(const char* fmt, ...);
int printk_warn(const char* fmt, ...);
int printk_err(const char* fmt, ...);

}

#endif
