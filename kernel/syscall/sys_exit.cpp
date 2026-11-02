// DIT-OS Kernel - sys_exit

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include "sched/wait.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_exit_impl(const SyscallArgs* args);

SyscallResult sys_exit_impl(const SyscallArgs* args)
{
    /* Only the low 8 bits are part of the POSIX status; waitpid() returns the
     * encoded word (WIFEXITED/WEXITSTATUS). */
    int code = static_cast<int>(args->args[0] & 0xFFu);
    /* Terminate current task and schedule next. */
    sched::task_exit(sched::make_wait_status_exit(code));
    /* Should never return. */
    for (;;)
        asm volatile("hlt");
}

} // namespace syscall
