// DIT-OS Kernel - Syscall wrappers (for kernel threads calling syscalls without int 0x80)

#include "syscall/syscall.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

namespace syscall {

int syscall_read(int fd, void* buf, uint32_t count) {
    SyscallArgs args;
    args.args[0] = fd;
    args.args[1] = (uintptr_t)buf;
    args.args[2] = count;
    args.args[3] = 0;
    args.args[4] = 0;
    args.args[5] = 0;
    SyscallResult r = handle(SYS_read, &args);
    return (int)r;
}

int syscall_write(int fd, const void* buf, uint32_t count) {
    SyscallArgs args;
    args.args[0] = fd;
    args.args[1] = (uintptr_t)buf;
    args.args[2] = count;
    args.args[3] = 0;
    args.args[4] = 0;
    args.args[5] = 0;
    SyscallResult r = handle(SYS_write, &args);
    return (int)r;
}

void syscall_exit(int code) {
    SyscallArgs args;
    args.args[0] = (uint32_t)code;
    args.args[1] = 0;
    args.args[2] = 0;
    args.args[3] = 0;
    args.args[4] = 0;
    args.args[5] = 0;
    handle(SYS_exit, &args);
}

} // namespace syscall
