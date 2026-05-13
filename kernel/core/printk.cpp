// DIT-OS Kernel - printf-style output (printk)

#include "core/printk.hpp"
#include "core/compiler.hpp"
#include "dev/serial.hpp"
#include <stdint.h>

namespace core
{

static void putchar(char c)
{
    dev::serial_putchar(c);
    if (c == '\n')
        dev::serial_putchar('\r');
}

static void putchar_vga(char c)
{
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
    static int cx = 0, cy = 0;
    const int W = 80, H = 25;
    if (c == '\n') {
        cy++;
        cx = 0;
    } else if (c == '\r') {
        cx = 0;
    } else {
        if (cx < W) {
            vga[cy * W + cx] = (uint16_t)(uint8_t)c | (0x0F << 8);
            cx++;
        }
        if (cx >= W) {
            cx = 0;
            cy++;
        }
    }
    if (cy >= H) {
        for (int y = 0; y < H - 1; y++)
            for (int x = 0; x < W; x++)
                vga[y * W + x] = vga[(y + 1) * W + x];
        for (int x = 0; x < W; x++)
            vga[(H - 1) * W + x] = 0x0F00;
        cy = H - 1;
    }
}

static unsigned int u32_to_dec(uint32_t val, char* buf, unsigned int max)
{
    if (max == 0)
        return 0;
    if (val == 0) {
        buf[0] = '0';
        return 1;
    }
    char tmp[12];
    unsigned int i = 0;
    while (val && i < sizeof(tmp)) {
        tmp[i++] = (char)('0' + (val % 10));
        val /= 10;
    }
    unsigned int n = i < max ? i : max;
    for (unsigned int j = 0; j < n; j++)
        buf[j] = tmp[i - 1 - j];
    return n;
}

static unsigned int u32_to_hex(uint32_t val, char* buf, unsigned int max)
{
    const char* hex = "0123456789ABCDEF";
    if (max < 8)
        return 0;
    for (int i = 7; i >= 0; i--)
        buf[7 - i] = hex[(val >> (i * 4)) & 0xF];
    return 8;
}

int vprintk(const char* fmt, va_list args)
{
    int n = 0;
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            putchar(*fmt);
            putchar_vga(*fmt);
            n++;
            continue;
        }
        fmt++;
        if (!*fmt)
            break;
        if (*fmt == '%') {
            putchar('%');
            putchar_vga('%');
            n++;
            continue;
        }
        if (*fmt == 'c') {
            int c = va_arg(args, int);
            putchar((char)c);
            putchar_vga((char)c);
            n++;
            continue;
        }
        if (*fmt == 's') {
            const char* s = va_arg(args, const char*);
            if (!s)
                s = "(null)";
            while (*s) {
                putchar(*s);
                putchar_vga(*s);
                n++;
                s++;
            }
            continue;
        }
        if (*fmt == 'd' || *fmt == 'i') {
            int32_t v = va_arg(args, int32_t);
            char buf[12];
            if (v < 0) {
                putchar('-');
                putchar_vga('-');
                n++;
                v = (uint32_t) - (int32_t)v;
            }
            unsigned int len = u32_to_dec((uint32_t)v, buf, sizeof(buf));
            for (unsigned int i = 0; i < len; i++) {
                putchar(buf[i]);
                putchar_vga(buf[i]);
                n++;
            }
            continue;
        }
        if (*fmt == 'u') {
            uint32_t v = va_arg(args, uint32_t);
            char buf[12];
            unsigned int len = u32_to_dec(v, buf, sizeof(buf));
            for (unsigned int i = 0; i < len; i++) {
                putchar(buf[i]);
                putchar_vga(buf[i]);
                n++;
            }
            continue;
        }
        if (*fmt == 'x' || *fmt == 'X') {
            uint32_t v = va_arg(args, uint32_t);
            char buf[8];
            u32_to_hex(v, buf, 8);
            for (int i = 0; i < 8; i++) {
                putchar(buf[i]);
                putchar_vga(buf[i]);
                n++;
            }
            continue;
        }
        if (*fmt == 'p') {
            putchar('0');
            putchar_vga('0');
            putchar('x');
            putchar_vga('x');
            n += 2;
            uint32_t v = (uint32_t)va_arg(args, unsigned long);
            char buf[8];
            u32_to_hex(v, buf, 8);
            for (int i = 0; i < 8; i++) {
                putchar(buf[i]);
                putchar_vga(buf[i]);
                n++;
            }
            continue;
        }
        putchar('%');
        putchar_vga('%');
        putchar(*fmt);
        putchar_vga(*fmt);
        n += 2;
    }
    return n;
}

int printk(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vprintk(fmt, args);
    va_end(args);
    return n;
}

int printk_info(const char* fmt, ...)
{
    core::printk("[INFO] ");
    va_list args;
    va_start(args, fmt);
    int n = vprintk(fmt, args);
    va_end(args);
    return n;
}

int printk_warn(const char* fmt, ...)
{
    core::printk("[WARN] ");
    va_list args;
    va_start(args, fmt);
    int n = vprintk(fmt, args);
    va_end(args);
    return n;
}

int printk_err(const char* fmt, ...)
{
    core::printk("[ERR] ");
    va_list args;
    va_start(args, fmt);
    int n = vprintk(fmt, args);
    va_end(args);
    return n;
}

} // namespace core
