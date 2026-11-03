// DIT-OS Kernel - sys_close

#include "syscall/syscall_defs.hpp"
#include "syscall/fd.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_close_impl(const SyscallArgs* args);

SyscallResult sys_close_impl(const SyscallArgs* args)
{
    /* 0x1_0000_0001 must not truncate into fd 1. */
    if (args->args[0] >= static_cast<uint64_t>(FD_TABLE_SIZE))
        return -EBADF;
    return fd_close(static_cast<int>(args->args[0]));
}

} // namespace syscall
