#include "dev/serial.hpp"
#include "arch/x86_64/io.hpp"

namespace dev
{

static constexpr uint16_t COM1 = 0x3F8;
static constexpr uint16_t COM2 = 0x2F8;

static uint16_t base(int port)
{
    return port == 0 ? COM1 : COM2;
}

void serial_init()
{
    for (int p = 0; p < 2; p++) {
        uint16_t b = base(p);
        arch::outb(b + 1, 0);
        arch::outb(b + 3, 0x80);
        arch::outb(b + 0, 0x01);
        arch::outb(b + 1, 0);
        arch::outb(b + 3, 0x03);
        arch::outb(b + 2, 0x01);
        arch::outb(b + 4, 0);
    }
}

void serial_putchar(int port, char c)
{
    uint16_t b = base(port);
    while (!(arch::inb(b + 5) & 0x20)) {
    }
    arch::outb(b + 0, (unsigned char)c);
}

void serial_puts(int port, const char* str)
{
    while (*str) {
        serial_putchar(port, *str);
        if (*str == '\n')
            serial_putchar(port, '\r');
        str++;
    }
}

int serial_has_input(int port)
{
    return (arch::inb(base(port) + 5) & 0x01) ? 1 : 0;
}

int serial_getchar(int port)
{
    if (!serial_has_input(port))
        return -1;
    return arch::inb(base(port) + 0) & 0xFF;
}

void serial_putchar(char c)
{
    serial_putchar(0, c);
}
void serial_puts(const char* str)
{
    serial_puts(0, str);
}

} // namespace dev
