// DIT-OS Kernel - sys_exit stub

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include <stdint.h>

namespace syscall {

extern SyscallResult sys_exit_impl(const SyscallArgs* args);

SyscallResult sys_exit_impl(const SyscallArgs* args) {
    int code = (int)args->args[0];
    /* Terminate current task and schedule next. */
    sched::task_exit(code);
    /* Should never return. */
    for (;;) asm volatile("hlt");
}

} // namespace syscall
