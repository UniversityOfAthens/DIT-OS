// DIT-OS Kernel - sys_open (VFS)

#include "errno_defs.hpp"
#include "fs/vfs.hpp"
#include "syscall/fd.hpp"
#include "syscall/syscall_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_open_impl(const SyscallArgs* args);

static int copy_user_path(uintptr_t uptr, char* out, uint32_t cap)
{
    if (!out || cap < 2)
        return -EINVAL;
    const volatile char* p = (const volatile char*)uptr;
    for (uint32_t i = 0; i < cap - 1; i++) {
        char c = p[i];
        out[i] = c;
        if (c == '\0')
            return 0;
    }
    out[cap - 1] = '\0';
    return -EINVAL;
}

SyscallResult sys_open_impl(const SyscallArgs* args)
{
    char path[256];
    int pe = copy_user_path(args->args[0], path, sizeof(path));
    if (pe != 0)
        return pe;

    fs::file* f = fs::vfs_open(path, 0);
    if (!f)
        return -ENOENT;

    int fd = fd_install(f);
    if (fd < 0) {
        fs::vfs_close(f);
        return fd;
    }
    return fd;
}

} // namespace syscall
