// DIT-OS Kernel - sys_fork

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_fork_impl(const SyscallArgs* args);

SyscallResult sys_fork_impl(const SyscallArgs* args)
{
    (void)args;
    return sched::fork_current();
}

} // namespace syscall
