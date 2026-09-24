#ifndef KERNEL_GRAPHICS_TERMINAL_HPP
#define KERNEL_GRAPHICS_TERMINAL_HPP

#include <stdint.h>
#include "graphics/terminal-buffer.hpp"

namespace gfx
{

/** Kernel terminal: VGA output + optional scrollback buffer */
struct Terminal
{
    TerminalBuffer* buffer; /* nullptr if no scrollback */
    uint8_t attr; /* current attribute (fg|bg<<4) */
};

/** Initialize default kernel terminal (VGA) */
void terminal_init();

/** Write string to current terminal */
void terminal_puts(const char* str);

/** Write character to current terminal */
void terminal_putc(char c);

/** Set current attribute (VGA: fg | (bg << 4)) */
void terminal_set_attr(uint8_t attr);

/** Get current terminal (for multi-console later) */
Terminal* terminal_current();

} // namespace gfx

#endif
