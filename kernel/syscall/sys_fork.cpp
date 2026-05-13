// DIT-OS Kernel - fork() (stub)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

SyscallResult sys_fork_impl(const SyscallArgs* args)
{
    (void)args;
    return -ENOSYS;
}

} // namespace syscall
