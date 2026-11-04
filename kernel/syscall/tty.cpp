// DIT-OS Kernel - Console TTY: modes, line discipline and job control (COM1)

#include "syscall/tty.hpp"
#include "dev/serial.hpp"
#include "fs/file.hpp"
#include "fs/ops.hpp"
#include "mm/kmalloc.hpp"
#include "sched/sched.hpp"
#include "sched/signal.hpp"
#include "sched/task.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

namespace
{

constexpr char KEY_BACKSPACE = '\b';
constexpr char KEY_DELETE = 0x7F;
/* ISIG characters (the termios defaults VINTR, VQUIT, VSUSP). */
constexpr uint8_t KEY_INTR = 0x03; // ^C
constexpr uint8_t KEY_QUIT = 0x1C; // ^backslash
constexpr uint8_t KEY_SUSP = 0x1A; // ^Z
/* The first process (init) leads group 1 and owns the console at boot. */
constexpr uint32_t INIT_PGRP = 1;
/* QEMU's ACPI PM1a control port and the S5 (soft-off) value. */
constexpr uint16_t QEMU_ACPI_PM1A_CNT = 0x604;
constexpr uint16_t QEMU_ACPI_SLP_EN_S5 = 0x2000;

uint32_t s_mode = TTY_MODE_DEFAULT;
/* Foreground process group: the one ^C reaches and that may read. Written
 * by tasks, read by the COM1 IRQ, so accesses happen with IRQs off. */
uint32_t s_fg_pgrp = INIT_PGRP;
sched::WaitQueue s_rx_wait; // readers waiting for console input

/* Canonical line buffer, owned by the TTY like Linux's line discipline: a
 * caught signal (SIGCHLD, SIGWINCH, ...) must not lose what was typed, so
 * the interrupted read returns -EINTR and the retry continues the same
 * line (L25). ISIG characters are the exception: POSIX flushes the input
 * queue (NOFLSH is not supported), so ^C/^\/^Z discard the partial line.
 * One buffer for the console: only the foreground group may read, and two
 * foreground readers would split lines between them. */
constexpr uint32_t TTY_LINE_MAX = 256;
static char s_line[TTY_LINE_MAX];
static uint32_t s_line_len;
static bool s_line_complete;

// COM1 IRQ context: new bytes are buffered, wake any blocked reader.
void tty_rx_ready()
{
    sched::wake_up_all(&s_rx_wait);
}

void tty_echo(char c)
{
    if (s_mode & TTY_MODE_ECHO)
        dev::serial_putchar(c);
}

void echo_control(char letter)
{
    tty_echo('^');
    tty_echo(letter);
    tty_echo('\r');
    tty_echo('\n');
}

// COM1 IRQ context: with ISIG, the job-control characters become signals to
// the foreground group instead of input.
bool tty_rx_filter(uint8_t byte)
{
    if (!(s_mode & TTY_MODE_ISIG))
        return false;
    int sig = 0;
    char letter = 0;
    if (byte == KEY_INTR)
    {
        sig = sched::SIGINT;
        letter = 'C';
    }
    else if (byte == KEY_QUIT)
    {
        sig = sched::SIGQUIT;
        letter = '\\';
    }
    else if (byte == KEY_SUSP)
    {
        sig = sched::SIGTSTP;
        letter = 'Z';
    }
    else
    {
        return false;
    }
    echo_control(letter);
    /* POSIX flushes the pending input on an ISIG character (NOFLSH is not
     * implemented): the partial line dies with the ^C, not with every
     * caught signal. Runs in IRQ context with interrupts already off. */
    s_line_len = 0;
    s_line_complete = false;
    sched::signal_send_group(s_fg_pgrp, sig);
    return true;
}

bool group_exists(uint32_t pgid)
{
    for (sched::Task* t = sched::task_list_head(); t; t = t->all_next)
    {
        if (t->pgid == pgid && t->state != sched::TaskState::Exited)
            return true;
    }
    return false;
}

/* Job control for a console access by the current task: a background
 * process group gets @sig (SIGTTIN for reads, SIGTTOU for SETPGRP) and,
 * stopped by its default action, retries once continued. Returns 0 when
 * the task may go ahead, -EIO when @sig is blocked or ignored (POSIX), or
 * -EINTR when a handler must run first. */
int job_control_check(int sig)
{
    sched::Task* t = sched::current_task;
    if (!t || !t->user_mode)
        return 0;
    for (;;)
    {
        uint32_t flags = arch::irq_save();
        bool foreground = t->pgid == s_fg_pgrp;
        if (foreground)
        {
            arch::irq_restore(flags);
            return 0;
        }
        if (sched::signal_blocked_or_ignored(t, sig))
        {
            arch::irq_restore(flags);
            /* SETPGRP from a background shell that ignores SIGTTOU is how
             * job control hands the terminal back; reads get -EIO. */
            return sig == sched::SIGTTOU ? 0 : -EIO;
        }
        uint32_t stops = t->stop_count;
        sched::signal_send_group(t->pgid, sig);
        /* Runs the stop right here; true means a handler is waiting. */
        bool caught = sched::signal_interrupt_pending();
        bool stopped = t->stop_count != stops;
        arch::irq_restore(flags);
        if (caught)
            return -EINTR;
        /* Not stopped (e.g. init, which discards default actions): retrying
         * would spin, so behave as if the signal were ignored. */
        if (!stopped)
            return sig == sched::SIGTTOU ? 0 : -EIO;
    }
}

// Sleep until the console has a byte and return it (0..255), or -EINTR when
// a caught signal arrives first. Checking the buffer and going to sleep
// happen with IRQs off, so a byte arriving in between still wakes us.
int tty_wait_byte()
{
    for (;;)
    {
        uint32_t flags = arch::irq_save();
        int c = dev::serial_rx_pop();
        bool interrupted = c < 0 && !sched::sleep_on(&s_rx_wait);
        arch::irq_restore(flags);
        if (c >= 0)
            return c;
        if (interrupted)
            return -EINTR;
    }
}

// Canonical mode: edit one line (backspace), return it with a trailing '\n'.
// The partial line lives in the TTY, not the reader's stack, so a caught
// signal returns -EINTR without dropping it; the retry continues the same
// line (L25).
int tty_read_line(char* buf, uint32_t count)
{
    if (count == 0)
        return 0;
    for (;;)
    {
        uint32_t flags = arch::irq_save();
        if (s_line_complete)
        {
            /* Hand out a completed line: a read shorter than the line
             * consumes what fits and the rest waits for the next read. */
            uint32_t n = s_line_len < count ? s_line_len : count;
            for (uint32_t i = 0; i < n; i++)
                buf[i] = s_line[i];
            uint32_t rest = s_line_len - n;
            for (uint32_t i = 0; i < rest; i++)
                s_line[i] = s_line[n + i];
            s_line_len = rest;
            if (rest == 0)
                s_line_complete = false;
            arch::irq_restore(flags);
            return static_cast<int>(n);
        }
        arch::irq_restore(flags);

        int got = tty_wait_byte();
        if (got < 0)
            return got; /* -EINTR: the partial line stays buffered (L25) */
        char ch = static_cast<char>(got);
        if (ch == KEY_BACKSPACE || ch == KEY_DELETE)
        {
            flags = arch::irq_save();
            bool erased = s_line_len > 0;
            if (erased)
                s_line_len--;
            arch::irq_restore(flags);
            if (erased)
            {
                tty_echo('\b');
                tty_echo(' ');
                tty_echo('\b');
            }
            continue;
        }
        bool newline = ch == '\r' || ch == '\n';
        flags = arch::irq_save();
        /* Keep one byte free so the '\n' always fits; a full line drops
         * further input until it is completed or erased. */
        bool room = s_line_len < (newline ? TTY_LINE_MAX : TTY_LINE_MAX - 1);
        if (room)
        {
            s_line[s_line_len++] = newline ? '\n' : ch;
            if (newline)
                s_line_complete = true;
        }
        arch::irq_restore(flags);
        if (!room)
            continue;
        if (newline)
        {
            tty_echo('\r');
            tty_echo('\n');
        }
        else
        {
            tty_echo(ch);
        }
    }
}

// Raw mode: block for the first byte, then take whatever else is already
// waiting (e.g. the rest of an escape sequence or pasted text).
int tty_read_raw(char* buf, uint32_t count)
{
    int first = tty_wait_byte();
    if (first < 0)
        return first;
    uint32_t n = 0;
    buf[n++] = static_cast<char>(first);
    tty_echo(buf[0]);
    while (n < count)
    {
        int c = dev::serial_rx_pop();
        if (c < 0)
            break;
        buf[n] = static_cast<char>(c);
        tty_echo(buf[n]);
        n++;
    }
    return static_cast<int>(n);
}

int console_read(fs::file* f, void* buf, uint32_t count)
{
    (void)f;
    if (count == 0)
        return 0;
    int access = job_control_check(sched::SIGTTIN);
    if (access < 0)
        return access;
    char* out = static_cast<char*>(buf);
    if (s_mode & TTY_MODE_CANON)
        return tty_read_line(out, count);
    return tty_read_raw(out, count);
}

int console_write(fs::file* f, const void* buf, uint32_t count)
{
    (void)f;
    const char* in = static_cast<const char*>(buf);
    for (uint32_t i = 0; i < count; i++)
        dev::serial_putchar(in[i]);
    return static_cast<int>(count);
}

fs::FileOps s_console_ops = {
    nullptr, // open
    console_read, console_write,
    nullptr, // release: nothing beyond the file itself
    nullptr, // readdir
};

} // namespace

void tty_init()
{
    dev::serial_enable_rx_irq(tty_rx_ready, tty_rx_filter);
}

fs::file* tty_open_console()
{
    fs::file* f = static_cast<fs::file*>(mm::kmalloc(sizeof(fs::file)));
    if (!f)
        return nullptr;
    uint8_t* raw = reinterpret_cast<uint8_t*>(f);
    for (uint32_t i = 0; i < sizeof(fs::file); i++)
        raw[i] = 0;
    f->f_refcount = 1;
    f->f_ops = &s_console_ops;
    return f;
}

bool tty_is_console(const fs::file* f)
{
    return f && f->f_ops == &s_console_ops;
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

int tty_ioctl(uint32_t request, uint32_t arg)
{
    if (request == TTY_IOC_GETMODE)
        return static_cast<int>(tty_get_mode());
    if (request == TTY_IOC_SETMODE)
        return tty_set_mode(arg);
    if (request == TTY_IOC_GETPGRP)
    {
        uint32_t flags = arch::irq_save();
        uint32_t pgrp = s_fg_pgrp;
        arch::irq_restore(flags);
        return static_cast<int>(pgrp);
    }
    if (request == TTY_IOC_SETPGRP)
    {
        /* Group ids are positive 32-bit values naming a live group: the
         * terminal cannot be handed to a group nobody is in. */
        if (arg == 0 || arg > static_cast<uint32_t>(INT32_MAX))
            return -EINVAL;
        int access = job_control_check(sched::SIGTTOU);
        if (access < 0)
            return access;
        uint32_t flags = arch::irq_save();
        if (!group_exists(arg))
        {
            arch::irq_restore(flags);
            return -EPERM;
        }
        s_fg_pgrp = arg;
        arch::irq_restore(flags);
        return 0;
    }
    if (request == TTY_IOC_POWER_OFF)
    {
        if (sched::current_pid() != 1)
            return -EPERM;
        __asm__ volatile("outw %0, %1"
                         :
                         : "a"(QEMU_ACPI_SLP_EN_S5), "Nd"(QEMU_ACPI_PM1A_CNT));
        for (;;)
            __asm__ volatile("hlt");
    }
    return -ENOTTY;
}

} // namespace syscall
