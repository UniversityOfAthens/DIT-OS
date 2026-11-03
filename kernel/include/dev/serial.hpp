#ifndef KERNEL_DEV_SERIAL_HPP
#define KERNEL_DEV_SERIAL_HPP

#include <stdint.h>

namespace dev
{

/** COM1 interrupt line on the 8259 PIC. */
constexpr int SERIAL_COM1_IRQ = 4;

void serial_init();
void serial_putchar(int port, char c);
void serial_puts(int port, const char* str);
void serial_putchar(char c);
void serial_puts(const char* str);

/** Enable COM1 receive interrupts; @handler runs in IRQ context after new
 * bytes are buffered. @filter (may be null) sees each byte first, in IRQ
 * context, and returns true to consume it instead of buffering it (the TTY
 * uses this for ^C/^Z). The caller must also unmask SERIAL_COM1_IRQ. */
void serial_enable_rx_irq(void (*handler)(), bool (*filter)(uint8_t byte));
/** COM1 IRQ handler: move received bytes into the RX buffer. */
void serial_irq();
/** Next buffered COM1 byte, or -1 if none (never blocks). */
int serial_rx_pop();

} // namespace dev

#endif
