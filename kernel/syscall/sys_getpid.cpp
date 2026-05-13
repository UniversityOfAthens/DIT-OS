// DIT-OS Kernel - getpid()

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include <stdint.h>

namespace syscall
{

SyscallResult sys_getpid_impl(const SyscallArgs* args)
{
    (void)args;
    return (SyscallResult)sched::current_pid();
}

} // namespace syscall
