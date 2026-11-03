#ifndef KERNEL_SYSCALL_TTY_HPP
#define KERNEL_SYSCALL_TTY_HPP

// DIT-OS Kernel - Console TTY (serial COM1): line discipline, job control

#include "fs/ioctl.hpp"
#include <stdint.h>

namespace fs
{
struct file;
}

namespace syscall
{

/** Mode flags (like termios ECHO / ICANON / ISIG). Default: all set. */
constexpr uint32_t TTY_MODE_ECHO = 1u << 0; // echo input back to the console
constexpr uint32_t TTY_MODE_CANON = 1u << 1; // line editing, read() per line
constexpr uint32_t TTY_MODE_ISIG = 1u << 2; // ^C ^\ ^Z raise signals
constexpr uint32_t TTY_MODE_MASK = TTY_MODE_ECHO | TTY_MODE_CANON |
                                   TTY_MODE_ISIG;
constexpr uint32_t TTY_MODE_DEFAULT = TTY_MODE_MASK;

/** ioctl requests on a console fd. Values mirrored in
 * userland/include/syscall.h. Arguments are values, not pointers: SETMODE
 * takes the new mode, SETPGRP the new foreground process group; GETMODE and
 * GETPGRP return theirs. */
constexpr uint32_t TTY_IOC_GETMODE = _IO('T', 1);
constexpr uint32_t TTY_IOC_SETMODE = _IO('T', 2);
constexpr uint32_t TTY_IOC_POWER_OFF = _IO('T', 3);
constexpr uint32_t TTY_IOC_GETPGRP = _IO('T', 4);
constexpr uint32_t TTY_IOC_SETPGRP = _IO('T', 5);
static_assert(TTY_IOC_GETMODE == 0x5401 && TTY_IOC_SETMODE == 0x5402 &&
                  TTY_IOC_POWER_OFF == 0x5403 && TTY_IOC_GETPGRP == 0x5404 &&
                  TTY_IOC_SETPGRP == 0x5405,
              "must match userland/include/syscall.h");

/** Hook the TTY up to COM1 receive interrupts (call once at boot). */
void tty_init();

/** A new open file for the console (refcount 1), or nullptr when out of
 *  memory. Reads honour the line discipline and job control; writes go to
 *  COM1. */
fs::file* tty_open_console();

/** True if @f is a console file. */
bool tty_is_console(const fs::file* f);

/** Console ioctl for the current task. Returns the result or -errno
 *  (-ENOTTY for an unknown request). */
int tty_ioctl(uint32_t request, uint32_t arg);

uint32_t tty_get_mode();
/** Returns 0, or -EINVAL for unknown mode bits. */
int tty_set_mode(uint32_t mode);

} // namespace syscall

#endif // KERNEL_SYSCALL_TTY_HPP
