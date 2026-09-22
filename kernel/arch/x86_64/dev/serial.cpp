// DIT-OS Kernel - 16550 UART driver (COM1/COM2)
//
// Output is polled. COM1 input is interrupt driven: the IRQ drains the UART
// into a ring buffer and notifies the registered handler (the console TTY).

#include "dev/serial.hpp"
#include "arch/x86_64/io.hpp"
#include <stdint.h>

namespace dev
{

static constexpr uint16_t COM1 = 0x3F8;
static constexpr uint16_t COM2 = 0x2F8;

// Register offsets from the port base
static constexpr uint16_t UART_DATA = 0; // RBR/THR (DLL when DLAB=1)
static constexpr uint16_t UART_IER = 1; // interrupt enable (DLH when DLAB=1)
static constexpr uint16_t UART_FCR = 2; // FIFO control
static constexpr uint16_t UART_LCR = 3; // line control
static constexpr uint16_t UART_MCR = 4; // modem control
static constexpr uint16_t UART_LSR = 5; // line status

static constexpr uint8_t IER_RX_AVAILABLE = 0x01;
static constexpr uint8_t LCR_DLAB = 0x80;
static constexpr uint8_t LCR_8N1 = 0x03;
static constexpr uint8_t FCR_ENABLE_CLEAR = 0x07; // enable, clear RX and TX
static constexpr uint8_t MCR_DTR = 0x01;
static constexpr uint8_t MCR_RTS = 0x02;
static constexpr uint8_t MCR_OUT2 = 0x08; // gates the UART IRQ line on PCs
static constexpr uint8_t LSR_DATA_READY = 0x01;
static constexpr uint8_t LSR_THR_EMPTY = 0x20;
static constexpr uint8_t BAUD_DIVISOR_115200 = 1;

// Single producer (IRQ) / single consumer (task) ring: head is written only
// by the IRQ, tail only by the reader, so no lock is needed on one CPU.
static constexpr uint32_t RX_BUF_SIZE = 256; // power of two
static volatile uint8_t s_rx_buf[RX_BUF_SIZE];
static volatile uint32_t s_rx_head;
static volatile uint32_t s_rx_tail;
static void (*s_rx_handler)();

static uint16_t base(int port)
{
    return port == 0 ? COM1 : COM2;
}

void serial_init()
{
    for (int p = 0; p < 2; p++) {
        uint16_t b = base(p);
        arch::outb(b + UART_IER, 0);
        arch::outb(b + UART_LCR, LCR_DLAB);
        arch::outb(b + UART_DATA, BAUD_DIVISOR_115200);
        arch::outb(b + UART_IER, 0);
        arch::outb(b + UART_LCR, LCR_8N1);
        arch::outb(b + UART_FCR, FCR_ENABLE_CLEAR);
        arch::outb(b + UART_MCR, MCR_DTR | MCR_RTS | MCR_OUT2);
    }
}

void serial_enable_rx_irq(void (*handler)())
{
    s_rx_handler = handler;
    arch::outb(COM1 + UART_IER, IER_RX_AVAILABLE);
}

void serial_irq()
{
    bool got = false;
    while (arch::inb(COM1 + UART_LSR) & LSR_DATA_READY) {
        uint8_t byte = arch::inb(COM1 + UART_DATA);
        uint32_t head = s_rx_head;
        if (head - s_rx_tail < RX_BUF_SIZE) { // drop input when full
            s_rx_buf[head % RX_BUF_SIZE] = byte;
            s_rx_head = head + 1;
        }
        got = true;
    }
    if (got && s_rx_handler)
        s_rx_handler();
}

int serial_rx_pop()
{
    uint32_t tail = s_rx_tail;
    if (tail == s_rx_head)
        return -1;
    int byte = s_rx_buf[tail % RX_BUF_SIZE];
    s_rx_tail = tail + 1;
    return byte;
}

void serial_putchar(int port, char c)
{
    uint16_t b = base(port);
    while (!(arch::inb(b + UART_LSR) & LSR_THR_EMPTY)) {
    }
    arch::outb(b + UART_DATA, (unsigned char)c);
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

void serial_putchar(char c)
{
    serial_putchar(0, c);
}
void serial_puts(const char* str)
{
    serial_puts(0, str);
}

} // namespace dev
