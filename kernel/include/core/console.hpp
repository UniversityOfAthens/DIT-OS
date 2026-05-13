#ifndef KERNEL_CORE_CONSOLE_HPP
#define KERNEL_CORE_CONSOLE_HPP

namespace core
{

/** Initialize kernel console (serial + VGA). */
void console_init();

/** Write a character to the kernel console. */
void console_putchar(char c);

/** Write a null-terminated string. */
void console_puts(const char* str);

/** Set console output to serial only, VGA only, or both. */
enum class ConsoleTarget { Serial, VGA, Both };
void console_set_target(ConsoleTarget t);

} // namespace core

#endif
