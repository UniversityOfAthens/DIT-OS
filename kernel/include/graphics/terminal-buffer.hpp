#ifndef KERNEL_GRAPHICS_TERMINAL_BUFFER_HPP
#define KERNEL_GRAPHICS_TERMINAL_BUFFER_HPP

#include <stdint.h>

namespace gfx
{

/** Line in a terminal buffer (fixed max length) */
#define TERM_LINE_MAX 256

struct TermLine {
    char data[TERM_LINE_MAX];
    uint16_t len;
    uint8_t attr; /* default attribute for the line */
};

/** Scrollback buffer: fixed number of lines */
#define TERM_SCROLLBACK_LINES 512

struct TerminalBuffer {
    TermLine lines[TERM_SCROLLBACK_LINES];
    uint32_t head; /* index of first logical line */
    uint32_t count; /* number of lines in use */
    uint32_t cursor_row;
    uint32_t cursor_col;
    uint8_t default_attr;
};

/** Append a character to the buffer; handles newline and scroll */
void term_buf_putc(TerminalBuffer* buf, char c, uint8_t attr);

/** Clear the buffer */
void term_buf_clear(TerminalBuffer* buf);

/** Get line at logical index 0 = oldest, count-1 = newest */
const TermLine* term_buf_get_line(const TerminalBuffer* buf, uint32_t index);

} // namespace gfx

#endif
