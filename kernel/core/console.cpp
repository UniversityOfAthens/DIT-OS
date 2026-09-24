// DIT-OS Kernel - Console system

#include "core/console.hpp"
#include "dev/serial.hpp"
#include <stdint.h>

namespace core
{

static ConsoleTarget s_target = ConsoleTarget::Both;

void console_init()
{
    s_target = ConsoleTarget::Both;
}

static void vga_putchar(char c)
{
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    static int cx = 0, cy = 0;
    const int W = 80, H = 25;
    if (c == '\n')
    {
        cy++;
        cx = 0;
    }
    else if (c == '\r')
    {
        cx = 0;
    }
    else
    {
        if (cx < W)
        {
            vga[cy * W + cx] = (uint16_t)(uint8_t)c | (0x0F << 8);
            cx++;
        }
        if (cx >= W)
        {
            cx = 0;
            cy++;
        }
    }
    if (cy >= H)
    {
        for (int y = 0; y < H - 1; y++)
            for (int x = 0; x < W; x++)
                vga[y * W + x] = vga[(y + 1) * W + x];
        for (int x = 0; x < W; x++)
            vga[(H - 1) * W + x] = 0x0F00;
        cy = H - 1;
    }
}

void console_putchar(char c)
{
    if (s_target == ConsoleTarget::Serial || s_target == ConsoleTarget::Both)
        dev::serial_putchar(c);
    if (c == '\n' &&
        (s_target == ConsoleTarget::Serial || s_target == ConsoleTarget::Both))
        dev::serial_putchar('\r');
    if (s_target == ConsoleTarget::VGA || s_target == ConsoleTarget::Both)
        vga_putchar(c);
}

void console_puts(const char* str)
{
    while (*str)
    {
        console_putchar(*str);
        str++;
    }
}

void console_set_target(ConsoleTarget t)
{
    s_target = t;
}

} // namespace core
