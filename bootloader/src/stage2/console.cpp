// =============================================================================
// Boot console output - VGA text
// =============================================================================

#include "../../include/video.hpp"
#include "../../include/types.hpp"

namespace boot
{

static int s_cursor_x = 0;
static int s_cursor_y = 0;

void console_putchar(char c)
{
    volatile u16* vga = (volatile u16*)VGA_TEXT_BASE;
    if (c == '\n') {
        s_cursor_x = 0;
        s_cursor_y++;
    } else {
        u16 attr = (0x0F << 8) | (u8)c;
        vga[s_cursor_y * VGA_COLS + s_cursor_x] = attr;
        s_cursor_x++;
        if (s_cursor_x >= VGA_COLS) {
            s_cursor_x = 0;
            s_cursor_y++;
        }
    }
    if (s_cursor_y >= VGA_ROWS) {
        s_cursor_y = VGA_ROWS - 1;
        for (int y = 0; y < VGA_ROWS - 1; y++)
            for (int x = 0; x < VGA_COLS; x++)
                vga[y * VGA_COLS + x] = vga[(y + 1) * VGA_COLS + x];
    }
}

void console_write(const char* str)
{
    while (*str)
        console_putchar(*str++);
}

void console_clear()
{
    volatile u16* vga = (volatile u16*)VGA_TEXT_BASE;
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++)
        vga[i] = (0x0F << 8) | ' ';
    s_cursor_x = s_cursor_y = 0;
}

} // namespace boot
