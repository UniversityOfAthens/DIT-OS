#ifndef KERNEL_SERIAL_H
#define KERNEL_SERIAL_H

#include "../types.h"
#include "io.h"

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8
#define SERIAL_COM3 0x3E8
#define SERIAL_COM4 0x2E8

void serial_init(u16 port);
void serial_write(u16 port, u8 data);
void serial_putchar(u16 port, char c);
void serial_write_string(u16 port, const char *str);
int serial_is_ready(u16 port);

#endif
