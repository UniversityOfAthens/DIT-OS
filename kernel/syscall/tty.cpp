// DIT-OS Kernel - Console TTY: stdin modes and line discipline (COM1)

#include "syscall/tty.hpp"
#include "dev/serial.hpp"
#include "sched/sched.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

constexpr int TTY_FD_LAST = 2; // stdin, stdout, stderr
constexpr char KEY_BACKSPACE = '\b';
constexpr char KEY_DELETE = 0x7F;

static uint32_t s_mode = TTY_MODE_DEFAULT;

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

// Wait until the console has a byte and return it.
static char tty_wait_byte()
{
    for (;;) {
        int c = dev::serial_getchar(0);
        if (c >= 0)
            return static_cast<char>(c & 0xFF);
        sched::maybe_yield();
        __asm__ volatile("pause");
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
        int c = dev::serial_getchar(0);
        if (c < 0)
            break;
        buf[n] = static_cast<char>(c & 0xFF);
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
