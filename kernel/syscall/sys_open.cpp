// DIT-OS Kernel - sys_open (VFS)

#include "errno_defs.hpp"
#include "fs/vfs.hpp"
#include "mm/uaccess.hpp"
#include "syscall/fd.hpp"
#include "syscall/path.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_open_impl(const SyscallArgs* args);

SyscallResult sys_open_impl(const SyscallArgs* args)
{
    char path[PATH_MAX];
    int len = path_from_user(path, sizeof(path),
                             static_cast<uintptr_t>(args->args[0]));
    if (len < 0)
        return len;

    fs::file* f = fs::vfs_open(path, 0);
    if (!f)
        return -ENOENT;

    int fd = fd_install(f);
    if (fd < 0)
    {
        fs::vfs_close(f);
        return fd;
    }
    return fd;
}

} // namespace syscall
