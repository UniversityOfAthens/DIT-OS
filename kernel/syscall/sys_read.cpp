// DIT-OS Kernel - sys_read (fd 0 -> console TTY, fds >= 3 -> VFS)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "syscall/tty.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_read_impl(const SyscallArgs* args);

SyscallResult sys_read_impl(const SyscallArgs* args)
{
    int fd = (int)args->args[0];
    uintptr_t buf = (uintptr_t)args->args[1];
    uint32_t count = (uint32_t)args->args[2];
    if (count == 0)
        return 0;
    if (fd == 0)
        return tty_read(reinterpret_cast<char*>(buf), count);
    fs::file* vf = fd_get(fd);
    if (vf)
        return fs::vfs_read(vf, (void*)buf, count);
    return -EBADF;
}

} // namespace syscall
