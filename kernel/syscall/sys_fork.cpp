// DIT-OS Kernel - fork() (stub)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include <stdint.h>

namespace syscall {

SyscallResult sys_fork_impl(const SyscallArgs* args) {
    (void)args;
    return -38; /* -ENOSYS */
}

} // namespace syscall

