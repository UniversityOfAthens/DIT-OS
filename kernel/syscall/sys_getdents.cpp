// DIT-OS Kernel - sys_getdents (read directory entries into user buffer)

#include "errno_defs.hpp"
#include "fs/dirent.hpp"
#include "fs/vfs.hpp"
#include "mm/uaccess.hpp"
#include "syscall/fd.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_getdents_impl(const SyscallArgs* args);

SyscallResult sys_getdents_impl(const SyscallArgs* args)
{
    int fd = (int)args->args[0];
    uintptr_t buf = (uintptr_t)args->args[1];
    uint32_t count = (uint32_t)args->args[2];
    if (count == 0)
        return 0;

    if (!mm::user_range_ok(buf, count))
        return -EFAULT;
    fs::file* f = fd_get(fd);
    if (!f)
        return -EBADF;

    uint32_t off = 0;
    while (off + sizeof(fs::dirent) <= count) {
        fs::dirent de;
        int r = fs::vfs_readdir(f, &de);
        if (r < 0)
            return r;
        if (r == 0)
            break;
        if (mm::copy_to_user(buf + off, &de, sizeof(de)) < 0)
            return -EFAULT;
        off += sizeof(fs::dirent);
    }
    return (SyscallResult)off;
}

} // namespace syscall
