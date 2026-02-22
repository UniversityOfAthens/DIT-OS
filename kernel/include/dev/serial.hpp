#ifndef KERNEL_DEV_SERIAL_HPP
#define KERNEL_DEV_SERIAL_HPP

namespace dev {

void serial_init();
void serial_putchar(int port, char c);
void serial_puts(int port, const char *str);
int serial_getchar(int port);
int serial_has_input(int port);
void serial_putchar(char c);
void serial_puts(const char *str);

}
#endif
