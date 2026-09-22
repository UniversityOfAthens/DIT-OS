// DIT-OS Kernel - Syscall init and handle

#include "syscall/syscall.hpp"
#include "syscall/syscall_table.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_trace.hpp"
#include "syscall/fd.hpp"
#include "syscall/tty.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_read_impl(const SyscallArgs* args);
extern SyscallResult sys_write_impl(const SyscallArgs* args);
extern SyscallResult sys_exit_impl(const SyscallArgs* args);
extern SyscallResult sys_getpid_impl(const SyscallArgs* args);
extern SyscallResult sys_fork_impl(const SyscallArgs* args);
extern SyscallResult sys_open_impl(const SyscallArgs* args);
extern SyscallResult sys_close_impl(const SyscallArgs* args);
extern SyscallResult sys_getdents_impl(const SyscallArgs* args);
extern SyscallResult sys_ioctl_impl(const SyscallArgs* args);

void init()
{
    fd_init();
    tty_init();
    syscall_set_handler(SYS_read, sys_read_impl);
    syscall_set_handler(SYS_write, sys_write_impl);
    syscall_set_handler(SYS_exit, sys_exit_impl);
    syscall_set_handler(SYS_getpid, sys_getpid_impl);
    syscall_set_handler(SYS_fork, sys_fork_impl);
    syscall_set_handler(SYS_open, sys_open_impl);
    syscall_set_handler(SYS_close, sys_close_impl);
    syscall_set_handler(SYS_getdents, sys_getdents_impl);
    syscall_set_handler(SYS_ioctl, sys_ioctl_impl);
}

SyscallResult handle(uint32_t num, const SyscallArgs* args)
{
    trace_enter(num, args);
    SyscallResult r = syscall_dispatch(num, args);
    trace_exit(num, r);
    return r;
}

/** Called from IDT when int 0x80; regs = pointer to saved pusha block (low to
 * high: EDI,ESI,EBP,ESP,EBX,EDX,ECX,EAX) */
extern "C" void syscall_entry(uint32_t* regs)
{
    SyscallArgs args;
    args.args[0] = regs[4]; /* EBX */
    args.args[1] = regs[6]; /* ECX */
    args.args[2] = regs[5]; /* EDX */
    args.args[3] = regs[1]; /* ESI */
    args.args[4] = regs[0]; /* EDI */
    args.args[5] = 0;
    uint32_t num = regs[7]; /* EAX = syscall number */
    SyscallResult r = handle(num, &args);
    regs[7] = (uint32_t)r;
}

} // namespace syscall
