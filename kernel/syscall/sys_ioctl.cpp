// DIT-OS Kernel - sys_ioctl (console TTY mode only for now)

#include "syscall/syscall_defs.hpp"
#include "syscall/fd.hpp"
#include "syscall/tty.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_ioctl_impl(const SyscallArgs* args);

SyscallResult sys_ioctl_impl(const SyscallArgs* args)
{
    int fd = static_cast<int>(args->args[0]);
    uint32_t request = static_cast<uint32_t>(args->args[1]);
    uint32_t arg = static_cast<uint32_t>(args->args[2]);

    if (!tty_is_console_fd(fd))
        return fd_get(fd) ? -ENOTTY : -EBADF;

    if (request == TTY_IOC_GETMODE)
        return tty_get_mode();
    if (request == TTY_IOC_SETMODE)
        return tty_set_mode(arg);
    return -ENOTTY;
}

} // namespace syscall
