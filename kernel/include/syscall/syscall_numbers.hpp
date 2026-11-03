#ifndef KERNEL_SYSCALL_SYSCALL_NUMBERS_HPP
#define KERNEL_SYSCALL_SYSCALL_NUMBERS_HPP

#include <stdint.h>

namespace syscall
{

/** Syscall numbers (match syscall_table and arch-specific trap dispatch).
 *  Canonical values are documented in `.cursor/rules/syscall-abi.mdc`.
 */
enum SyscallNumber : uint32_t
{
    SYS_read = 0,
    SYS_write = 1,
    SYS_open = 2,
    SYS_close = 3,
    SYS_stat = 4,
    SYS_fstat = 5,
    SYS_exit = 6,

    SYS_fork = 7,
    SYS_execve = 8,
    SYS_waitpid = 9,
    SYS_getpid = 10,
    SYS_mmap = 11,
    SYS_munmap = 12,
    SYS_brk = 13,
    SYS_kill = 14,
    SYS_ioctl = 15,

    SYS_getdents = 16,
    SYS_sigaction = 17,
    SYS_sigprocmask = 18,
    SYS_sigpending = 19,
    SYS_sigreturn = 20,
    SYS_sigsuspend = 21,
    SYS_setpgid = 22,
    SYS_getpgid = 23,
    SYS_pipe = 24,
    SYS_dup = 25,
    SYS_dup2 = 26,
    SYS_chdir = 27,
    SYS_getcwd = 28,

    SYS_MAX = 29
};

} // namespace syscall

#endif
