#ifndef KERNEL_SYSCALL_TTY_HPP
#define KERNEL_SYSCALL_TTY_HPP

// DIT-OS Kernel - Console TTY (serial) behind fds 0-2

#include "fs/ioctl.hpp"
#include <stdint.h>

namespace syscall
{

/** Mode flags (like termios ECHO / ICANON). Default: both set. */
constexpr uint32_t TTY_MODE_ECHO = 1u << 0; // echo input back to the console
constexpr uint32_t TTY_MODE_CANON = 1u << 1; // line editing, read() per line
constexpr uint32_t TTY_MODE_MASK = TTY_MODE_ECHO | TTY_MODE_CANON;
constexpr uint32_t TTY_MODE_DEFAULT = TTY_MODE_ECHO | TTY_MODE_CANON;

/** ioctl requests on fds 0-2. Values mirrored in userland/include/syscall.h.
 * GETMODE returns the mode; SETMODE takes the new mode as the argument
 * value (not a pointer). */
constexpr uint32_t TTY_IOC_GETMODE = _IO('T', 1);
constexpr uint32_t TTY_IOC_SETMODE = _IO('T', 2);
static_assert(TTY_IOC_GETMODE == 0x5401 && TTY_IOC_SETMODE == 0x5402,
              "must match userland/include/syscall.h");

/** Hook the TTY up to COM1 receive interrupts (call once at boot). */
void tty_init();
/** True if @fd refers to the console TTY. */
bool tty_is_console_fd(int fd);
uint32_t tty_get_mode();
/** Returns 0, or -EINVAL for unknown mode bits. */
int tty_set_mode(uint32_t mode);
/** Blocking read from the console. Canonical mode returns one edited line;
 * raw mode returns as soon as at least one byte is available. */
int tty_read(char* buf, uint32_t count);

} // namespace syscall

#endif // KERNEL_SYSCALL_TTY_HPP
