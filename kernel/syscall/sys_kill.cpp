// DIT-OS Kernel - sys_kill (signal delivery with default actions)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/signal.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_kill_impl(const SyscallArgs* args);

SyscallResult sys_kill_impl(const SyscallArgs* args)
{
    /* Validate both arguments while they are still 64-bit: a value such as
     * 0x1_0000_0001 must not truncate into a valid pid, and a negative signal
     * must not wrap into a valid number. Process groups (pid <= 0) are not
     * implemented, so they are rejected too. */
    if (args->args[1] >= static_cast<uint64_t>(sched::NSIG))
        return -EINVAL;
    if (args->args[0] > static_cast<uint64_t>(INT32_MAX))
        return -EINVAL;

    int32_t pid = static_cast<int32_t>(args->args[0]);
    int sig = static_cast<int>(args->args[1]);
    return sched::signal_send_pid(pid, sig);
}

} // namespace syscall
