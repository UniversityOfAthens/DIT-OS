// DIT-OS Kernel - C ABI wrappers for userland (temporary; bypass int 0x80)

#include "syscall/syscall.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

extern "C" {

int dit_sys_read(int fd, void* buf, unsigned count)
{
    return syscall::syscall_read(fd, buf, (uint32_t)count);
}

int dit_sys_write(int fd, const void* buf, unsigned count)
{
    return syscall::syscall_write(fd, buf, (uint32_t)count);
}

void dit_sys_exit(int code)
{
    syscall::syscall_exit(code);
    for (;;)
        asm volatile("hlt");
}

int dit_sys_getpid(void)
{
    syscall::SyscallArgs args{};
    syscall::SyscallResult r = syscall::handle(syscall::SYS_getpid, &args);
    return (int)r;
}

int dit_sys_open(const char* path, int flags)
{
    syscall::SyscallArgs args{};
    args.args[0] = (uintptr_t)path;
    args.args[1] = (uint32_t)flags;
    args.args[2] = 0;
    syscall::SyscallResult r = syscall::handle(syscall::SYS_open, &args);
    return (int)r;
}

int dit_sys_close(int fd)
{
    syscall::SyscallArgs args{};
    args.args[0] = (uint32_t)fd;
    syscall::SyscallResult r = syscall::handle(syscall::SYS_close, &args);
    return (int)r;
}

int dit_sys_ioctl(int fd, unsigned long request, unsigned long arg)
{
    syscall::SyscallArgs args{};
    args.args[0] = (uint32_t)fd;
    args.args[1] = request;
    args.args[2] = arg;
    syscall::SyscallResult r = syscall::handle(syscall::SYS_ioctl, &args);
    return (int)r;
}

int dit_sys_getdents(int fd, void* buf, unsigned count)
{
    syscall::SyscallArgs args{};
    args.args[0] = (uint32_t)fd;
    args.args[1] = (uintptr_t)buf;
    args.args[2] = count;
    syscall::SyscallResult r = syscall::handle(syscall::SYS_getdents, &args);
    return (int)r;
}

} // extern "C"
