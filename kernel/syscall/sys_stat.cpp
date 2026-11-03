// DIT-OS Kernel - sys_stat, sys_fstat (file metadata)

#include "errno_defs.hpp"
#include "fs/vfs.hpp"
#include "mm/uaccess.hpp"
#include "syscall/fd.hpp"
#include "syscall/syscall_defs.hpp"
#include "syscall/tty.hpp"
#include "syscall/path.hpp"
#include "fs/pipe.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_stat_impl(const SyscallArgs* args);
extern SyscallResult sys_fstat_impl(const SyscallArgs* args);

/* Permission bits reported for inode-less files (crw--w---- / prw-------). */
constexpr uint32_t CONSOLE_MODE = 0620;
constexpr uint32_t PIPE_MODE = 0600;

SyscallResult sys_stat_impl(const SyscallArgs* args)
{
    uintptr_t user_stat = static_cast<uintptr_t>(args->args[1]);
    if (!mm::user_range_ok(user_stat, sizeof(fs::stat)))
        return -EFAULT;

    char path[PATH_MAX];
    int len = path_from_user(path, sizeof(path),
                             static_cast<uintptr_t>(args->args[0]));
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
    if (!f)
        return -EBADF;

    fs::stat st;
    if (f->f_inode)
    {
        fs::vfs_stat_inode(f->f_inode, &st);
    }
    else
    {
        /* The console and pipes have no inode: report only their type, and
         * zero the rest so no stack bytes reach user space. */
        uint8_t* raw = reinterpret_cast<uint8_t*>(&st);
        for (uint32_t i = 0; i < sizeof(st); i++)
            raw[i] = 0;
        if (tty_is_console(f))
            st.st_mode = S_IFCHR | CONSOLE_MODE;
        else if (fs::pipe_is_pipe(f))
            st.st_mode = S_IFIFO | PIPE_MODE;
    }

    if (mm::copy_to_user(user_stat, &st, sizeof(st)) < 0)
        return -EFAULT;
    return 0;
}

} // namespace syscall
