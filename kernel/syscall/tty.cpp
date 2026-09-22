// DIT-OS Kernel - Console TTY: stdin modes and line discipline (COM1)

#include "syscall/tty.hpp"
#include "dev/serial.hpp"
#include "sched/sched.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

constexpr int TTY_FD_LAST = 2; // stdin, stdout, stderr
constexpr char KEY_BACKSPACE = '\b';
constexpr char KEY_DELETE = 0x7F;

static uint32_t s_mode = TTY_MODE_DEFAULT;
static sched::WaitQueue s_rx_wait; // readers waiting for console input

// COM1 IRQ context: new bytes are buffered, wake any blocked reader.
static void tty_rx_ready()
{
    sched::wake_up_all(&s_rx_wait);
}

void tty_init()
{
    dev::serial_enable_rx_irq(tty_rx_ready);
}

bool tty_is_console_fd(int fd)
{
    return fd >= 0 && fd <= TTY_FD_LAST;
}

uint32_t tty_get_mode()
{
    return s_mode;
}

int tty_set_mode(uint32_t mode)
{
    if (mode & ~TTY_MODE_MASK)
        return -EINVAL;
    s_mode = mode;
    return 0;
}

// Sleep until the console has a byte and return it. Checking the buffer and
// going to sleep happen with IRQs off, so a byte arriving in between still
// wakes us.
static char tty_wait_byte()
{
    for (;;) {
        uint32_t flags = arch::irq_save();
        int c = dev::serial_rx_pop();
        if (c < 0)
            sched::sleep_on(&s_rx_wait);
        arch::irq_restore(flags);
        if (c >= 0)
            return static_cast<char>(c);
    }
}

static void tty_echo(char c)
{
    if (s_mode & TTY_MODE_ECHO)
        dev::serial_putchar(c);
}

// Canonical mode: edit one line (backspace), return it with a trailing '\n'.
static int tty_read_line(char* buf, uint32_t count)
{
    uint32_t n = 0;
    while (n < count) {
        char ch = tty_wait_byte();
        if (ch == KEY_BACKSPACE || ch == KEY_DELETE) {
            if (n > 0) {
                n--;
                tty_echo('\b');
                tty_echo(' ');
                tty_echo('\b');
            }
            continue;
        }
        if (ch == '\r' || ch == '\n') {
            buf[n++] = '\n';
            tty_echo('\r');
            tty_echo('\n');
            break;
        }
        buf[n++] = ch;
        tty_echo(ch);
    }
    return static_cast<int>(n);
}

// Raw mode: block for the first byte, then take whatever else is already
// waiting (e.g. the rest of an escape sequence or pasted text).
static int tty_read_raw(char* buf, uint32_t count)
{
    uint32_t n = 0;
    buf[n++] = tty_wait_byte();
    tty_echo(buf[0]);
    while (n < count) {
        int c = dev::serial_rx_pop();
        if (c < 0)
            break;
        buf[n] = static_cast<char>(c);
        tty_echo(buf[n]);
        n++;
    }
    return static_cast<int>(n);
}

int tty_read(char* buf, uint32_t count)
{
    if (count == 0)
        return 0;
    if (s_mode & TTY_MODE_CANON)
        return tty_read_line(buf, count);
    return tty_read_raw(buf, count);
}

} // namespace syscall
