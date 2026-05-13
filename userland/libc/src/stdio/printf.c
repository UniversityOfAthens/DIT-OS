// Minimal userland printf: supports %s, %d, %u, %x, %c, and %%

#include "stdio.h"
#include "syscall.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

static void putc(char c)
{
    write(1, &c, 1);
}

static void puts_cstr(const char* s)
{
    if (!s)
        s = "(null)";
    unsigned i = 0;
    while (s[i])
        i++;
    write(1, s, i);
}

static void utoa(uint32_t v, unsigned base, char* out, unsigned* out_len)
{
    static const char digits[] = "0123456789abcdef";
    char tmp[16];
    unsigned n = 0;
    if (v == 0)
        tmp[n++] = '0';
    while (v && n < sizeof(tmp)) {
        tmp[n++] = digits[v % base];
        v /= base;
    }
    unsigned k = 0;
    for (unsigned i = 0; i < n; i++)
        out[k++] = tmp[n - 1 - i];
    *out_len = k;
}

int printf(const char* fmt, ...)
{
    if (!fmt)
        return 0;
    va_list ap;
    va_start(ap, fmt);

    unsigned written = 0;
    char num_buf[16];
    unsigned num_len = 0;

    for (const char* p = fmt; *p; p++) {
        if (*p != '%') {
            putc(*p);
            written++;
            continue;
        }
        p++;
        if (!*p)
            break;

        if (*p == '%') {
            putc('%');
            written++;
            continue;
        }

        if (*p == 'c') {
            char c = (char)va_arg(ap, int);
            putc(c);
            written++;
            continue;
        }

        if (*p == 's') {
            const char* s = va_arg(ap, const char*);
            if (!s)
                s = "(null)";
            unsigned i = 0;
            while (s[i])
                i++;
            write(1, s, i);
            written += i;
            continue;
        }

        if (*p == 'd') {
            int v = va_arg(ap, int);
            uint32_t uv = (v < 0) ? (uint32_t)(-v) : (uint32_t)v;
            unsigned sign = (v < 0) ? 1 : 0;
            utoa(uv, 10, num_buf, &num_len);
            if (sign) {
                putc('-');
                written++;
            }
            write(1, num_buf, num_len);
            written += num_len;
            continue;
        }

        if (*p == 'u') {
            uint32_t v = va_arg(ap, unsigned);
            utoa(v, 10, num_buf, &num_len);
            write(1, num_buf, num_len);
            written += num_len;
            continue;
        }

        if (*p == 'x') {
            uint32_t v = va_arg(ap, unsigned);
            utoa(v, 16, num_buf, &num_len);
            write(1, num_buf, num_len);
            written += num_len;
            continue;
        }

        // Unknown specifier: print it literally.
        putc('%');
        putc(*p);
        written += 2;
    }

    va_end(ap);
    return (int)written;
}
