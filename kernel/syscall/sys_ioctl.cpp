// DIT-OS Kernel - sys_ioctl (console TTY requests)

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
    if (args->args[0] >= static_cast<uint64_t>(FD_TABLE_SIZE))
        return -EBADF;
    fs::file* f = fd_get(static_cast<int>(args->args[0]));
    if (!f)
        return -EBADF;
    if (!tty_is_console(f))
        return -ENOTTY;
    return tty_ioctl(static_cast<uint32_t>(args->args[1]),
                     static_cast<uint32_t>(args->args[2]));
}

} // namespace syscall
