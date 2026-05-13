// DIT-OS Kernel - sys_close

#include "syscall/syscall_defs.hpp"
#include "syscall/fd.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_close_impl(const SyscallArgs* args);

SyscallResult sys_close_impl(const SyscallArgs* args)
{
    int fd = (int)args->args[0];
    return fd_close(fd);
}

} // namespace syscall
