#include "arch/serial.h"
#include "kernel.h"

static u16 serial_port = SERIAL_COM1;

void serial_init(u16 port) {
    serial_port = port;

    outb(port + 1, 0x00);
    outb(port + 3, 0x80);
    outb(port + 0, 0x03);
    outb(port + 1, 0x00);
    outb(port + 3, 0x03);
    outb(port + 2, 0xC7);
    outb(port + 4, 0x0B);
    outb(port + 4, 0x1E);
    outb(port + 0, 0xAE);

    outb(port + 4, 0x0F);
}

int serial_is_ready(u16 port) { return inb(port + 5) & 0x20; }

void serial_write(u16 port, u8 data) {
    while (!serial_is_ready(port))
        ;
    outb(port, data);
}

void serial_putchar(u16 port, char c) { serial_write(port, (u8)c); }

void serial_write_string(u16 port, const char *str) {
    while (*str) {
        serial_putchar(port, *str);
        str++;
    }
}
