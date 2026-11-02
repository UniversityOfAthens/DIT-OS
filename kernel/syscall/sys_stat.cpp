// DIT-OS Kernel - sys_stat, sys_fstat (file metadata)

#include "errno_defs.hpp"
#include "fs/vfs.hpp"
#include "mm/uaccess.hpp"
#include "syscall/fd.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_stat_impl(const SyscallArgs* args);
extern SyscallResult sys_fstat_impl(const SyscallArgs* args);

SyscallResult sys_stat_impl(const SyscallArgs* args)
{
    uintptr_t user_stat = static_cast<uintptr_t>(args->args[1]);
    if (!mm::user_range_ok(user_stat, sizeof(fs::stat)))
        return -EFAULT;

    char path[256];
    int len = mm::strncpy_from_user(path, static_cast<uintptr_t>(args->args[0]),
                                    sizeof(path));
    if (len < 0)
        return len;

    fs::stat st;
    int r = fs::vfs_stat(path, &st);
    if (r < 0)
        return r;

    if (mm::copy_to_user(user_stat, &st, sizeof(st)) < 0)
        return -EFAULT;
    return 0;
}

SyscallResult sys_fstat_impl(const SyscallArgs* args)
{
    /* Arguments are 64-bit. A value such as 0x1_0000_0003 would truncate to a
     * valid fd when cast to int, so bound it while it is still 64-bit. */
    if (args->args[0] >= static_cast<uint64_t>(FD_TABLE_SIZE))
        return -EBADF;
    int fd = static_cast<int>(args->args[0]);
    uintptr_t user_stat = static_cast<uintptr_t>(args->args[1]);
    if (!mm::user_range_ok(user_stat, sizeof(fs::stat)))
        return -EFAULT;

    fs::file* f = fd_get(fd);
    if (!f || !f->f_inode)
        return -EBADF;

    fs::stat st;
    fs::vfs_stat_inode(f->f_inode, &st);

    if (mm::copy_to_user(user_stat, &st, sizeof(st)) < 0)
        return -EFAULT;
    return 0;
}

} // namespace syscall
