// DIT-OS Kernel - C ABI wrappers for userland (temporary; bypass int 0x80)

#include "syscall/syscall.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

extern "C" {

int dit_sys_read(int fd, void* buf, unsigned count) {
    return syscall::syscall_read(fd, buf, (uint32_t)count);
}

int dit_sys_write(int fd, const void* buf, unsigned count) {
    return syscall::syscall_write(fd, buf, (uint32_t)count);
}

void dit_sys_exit(int code) {
    syscall::syscall_exit(code);
    for (;;) asm volatile("hlt");
}

int dit_sys_getpid(void) {
    syscall::SyscallArgs args{};
    syscall::SyscallResult r = syscall::handle(syscall::SYS_getpid, &args);
    return (int)r;
}

} // extern "C"

