// DIT-OS Kernel - sys_write stub (fd 1/2 -> serial)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "dev/serial.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_write_impl(const SyscallArgs* args);

SyscallResult sys_write_impl(const SyscallArgs* args)
{
    int fd = (int)args->args[0];
    uintptr_t buf = (uintptr_t)args->args[1];
    uint32_t count = (uint32_t)args->args[2];
    if (count == 0)
        return 0;
    if (fd == 1 || fd == 2) {
        const char* p = (const char*)buf;
        for (uint32_t i = 0; i < count; i++)
            dev::serial_putchar(p[i]);
        return (SyscallResult)count;
    }
    fs::file* vf = fd_get(fd);
    if (vf)
        return fs::vfs_write(vf, (const void*)buf, count);
    return -EBADF;
}

} // namespace syscall
