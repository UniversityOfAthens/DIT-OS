#include "arch/serial.h"
#include <cstdarg>

extern u16 serial_port;

static char printk_buffer[256];

static void serial_puts(const char *str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar(serial_port, '\r');
        }
        serial_putchar(serial_port, *str);
        str++;
    }
}

void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char *buf = printk_buffer;
    const char *p = fmt;

    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
            case 'd': {
                int val = va_arg(args, int);
                char num_buf[16];
                int i = 0;
                if (val < 0) {
                    *buf++ = '-';
                    val = -val;
                }
                if (val == 0) {
                    *buf++ = '0';
                } else {
                    while (val > 0) {
                        num_buf[i++] = '0' + (val % 10);
                        val /= 10;
                    }
                    while (i > 0) {
                        *buf++ = num_buf[--i];
                    }
                }
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                while (*s)
                    *buf++ = *s++;
                break;
            }
            case 'x': {
                unsigned int val = va_arg(args, unsigned int);
                char hex_buf[16];
                int i = 0;
                if (val == 0) {
                    *buf++ = '0';
                } else {
                    while (val > 0) {
                        int d = val % 16;
                        hex_buf[i++] = (d < 10) ? ('0' + d) : ('a' + d - 10);
                        val /= 16;
                    }
                    *buf++ = '0';
                    *buf++ = 'x';
                    while (i > 0) {
                        *buf++ = hex_buf[--i];
                    }
                }
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                *buf++ = c;
                break;
            }
            case '%':
                *buf++ = '%';
                break;
            default:
                *buf++ = '%';
                *buf++ = *p;
                break;
            }
        } else {
            if (*p == '\n') {
                *buf++ = '\r';
            }
            *buf++ = *p;
        }
        p++;
    }

    *buf = '\0';
    va_end(args);

    serial_write_string(serial_port, printk_buffer);
}
