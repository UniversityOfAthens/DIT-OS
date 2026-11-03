// DIT-OS Kernel - Syscall init and handle

#include "syscall/syscall.hpp"
#include "syscall/syscall_table.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_trace.hpp"
#include "syscall/fd.hpp"
#include "syscall/tty.hpp"
#include "sched/signal.hpp"
#include "sched/sched.hpp"
#include "sched/wait.hpp"
#include "arch/x86_64/idt.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_read_impl(const SyscallArgs* args);
extern SyscallResult sys_write_impl(const SyscallArgs* args);
extern SyscallResult sys_exit_impl(const SyscallArgs* args);
extern SyscallResult sys_getpid_impl(const SyscallArgs* args);
extern SyscallResult sys_fork_impl(const SyscallArgs* args);
extern SyscallResult sys_waitpid_impl(const SyscallArgs* args);
extern SyscallResult sys_execve_impl(const SyscallArgs* args);
extern SyscallResult sys_brk_impl(const SyscallArgs* args);
extern SyscallResult sys_mmap_impl(const SyscallArgs* args);
extern SyscallResult sys_munmap_impl(const SyscallArgs* args);
extern SyscallResult sys_open_impl(const SyscallArgs* args);
extern SyscallResult sys_close_impl(const SyscallArgs* args);
extern SyscallResult sys_stat_impl(const SyscallArgs* args);
extern SyscallResult sys_fstat_impl(const SyscallArgs* args);
extern SyscallResult sys_getdents_impl(const SyscallArgs* args);
extern SyscallResult sys_ioctl_impl(const SyscallArgs* args);
extern SyscallResult sys_kill_impl(const SyscallArgs* args);
extern SyscallResult sys_sigaction_impl(const SyscallArgs* args);
extern SyscallResult sys_sigprocmask_impl(const SyscallArgs* args);
extern SyscallResult sys_sigpending_impl(const SyscallArgs* args);
extern SyscallResult sys_sigsuspend_impl(const SyscallArgs* args);
extern SyscallResult sys_setpgid_impl(const SyscallArgs* args);
extern SyscallResult sys_getpgid_impl(const SyscallArgs* args);
extern SyscallResult sys_pipe_impl(const SyscallArgs* args);
extern SyscallResult sys_dup_impl(const SyscallArgs* args);
extern SyscallResult sys_dup2_impl(const SyscallArgs* args);
extern SyscallResult sys_chdir_impl(const SyscallArgs* args);
extern SyscallResult sys_getcwd_impl(const SyscallArgs* args);

void init()
{
    fd_init();
    tty_init();
    syscall_set_handler(SYS_read, sys_read_impl);
    syscall_set_handler(SYS_write, sys_write_impl);
    syscall_set_handler(SYS_exit, sys_exit_impl);
    syscall_set_handler(SYS_getpid, sys_getpid_impl);
    syscall_set_handler(SYS_fork, sys_fork_impl);
    syscall_set_handler(SYS_kill, sys_kill_impl);
    syscall_set_handler(SYS_waitpid, sys_waitpid_impl);
    syscall_set_handler(SYS_execve, sys_execve_impl);
    syscall_set_handler(SYS_brk, sys_brk_impl);
    syscall_set_handler(SYS_mmap, sys_mmap_impl);
    syscall_set_handler(SYS_munmap, sys_munmap_impl);
    syscall_set_handler(SYS_open, sys_open_impl);
    syscall_set_handler(SYS_close, sys_close_impl);
    syscall_set_handler(SYS_stat, sys_stat_impl);
    syscall_set_handler(SYS_fstat, sys_fstat_impl);
    syscall_set_handler(SYS_getdents, sys_getdents_impl);
    syscall_set_handler(SYS_ioctl, sys_ioctl_impl);
    syscall_set_handler(SYS_sigaction, sys_sigaction_impl);
    syscall_set_handler(SYS_sigprocmask, sys_sigprocmask_impl);
    syscall_set_handler(SYS_sigpending, sys_sigpending_impl);
    syscall_set_handler(SYS_sigsuspend, sys_sigsuspend_impl);
    syscall_set_handler(SYS_setpgid, sys_setpgid_impl);
    syscall_set_handler(SYS_getpgid, sys_getpgid_impl);
    syscall_set_handler(SYS_pipe, sys_pipe_impl);
    syscall_set_handler(SYS_dup, sys_dup_impl);
    syscall_set_handler(SYS_dup2, sys_dup2_impl);
    syscall_set_handler(SYS_chdir, sys_chdir_impl);
    syscall_set_handler(SYS_getcwd, sys_getcwd_impl);
    /* SYS_sigreturn is handled in syscall_entry(): it rewrites the frame. */
}

SyscallResult handle(uint32_t num, const SyscallArgs* args)
{
    trace_enter(num, args);
    SyscallResult r = syscall_dispatch(num, args);
    trace_exit(num, r);
    return r;
}

static const uint32_t* s_syscall_regs;

const uint32_t* current_regs()
{
    return s_syscall_regs;
}

/** Called from IDT when int 0x80; regs = pointer to saved pusha block (low to
 * high: EDI,ESI,EBP,ESP,EBX,EDX,ECX,EAX) */
extern "C" void syscall_entry(uint32_t* regs)
{
    s_syscall_regs = regs;
    uint32_t num = regs[arch::FRAME_EAX];
    if (num == SYS_sigreturn)
    {
        /* sigreturn rewrites the whole saved context, EAX included, so it
         * bypasses the table: storing a result would clobber EAX. A forged
         * or corrupt frame is a fatal fault. */
        if (!sched::signal_sigreturn(regs))
            sched::task_exit(sched::make_wait_status_signal(sched::SIGSEGV));
        s_syscall_regs = nullptr;
        (void)sched::signal_handle_pending(regs);
        return;
    }

    SyscallArgs args;
    args.args[0] = regs[4]; /* EBX */
    args.args[1] = regs[6]; /* ECX */
    args.args[2] = regs[5]; /* EDX */
    args.args[3] = regs[1]; /* ESI */
    args.args[4] = regs[0]; /* EDI */
    args.args[5] = 0;
    SyscallResult r = handle(num, &args);
    s_syscall_regs = nullptr;
    /* An interrupted syscall whose handler has SA_RESTART runs again after
     * the handler: rewind to the int 0x80 with the number back in EAX (the
     * argument registers are preserved). sigsuspend() always reports EINTR. */
    if (r == -EINTR && num != SYS_sigsuspend && sched::signal_restart_pending())
    {
        regs[arch::FRAME_EIP] -= arch::SYSCALL_INSN_LEN;
        regs[arch::FRAME_EAX] = num;
    }
    else
    {
        regs[arch::FRAME_EAX] = static_cast<uint32_t>(r);
    }
    /* Signals act on the way back to user mode; a task that killed itself
     * does not get to run another instruction. */
    (void)sched::signal_handle_pending(regs);
}

} // namespace syscall
