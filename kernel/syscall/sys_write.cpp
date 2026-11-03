// DIT-OS Kernel - sys_write (any open file: console, VFS file or pipe)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "errno_defs.hpp"
#include "mm/uaccess.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_write_impl(const SyscallArgs* args)
{
    /* Bound the fd while still 64-bit (no truncation into a valid fd). */
    if (args->args[0] >= static_cast<uint64_t>(FD_TABLE_SIZE))
        return -EBADF;
    int fd = static_cast<int>(args->args[0]);
    uintptr_t buf = static_cast<uintptr_t>(args->args[1]);
    uint32_t count = static_cast<uint32_t>(args->args[2]);
    fs::file* vf = fd_get(fd);
    if (!vf)
        return -EBADF;
    if (count == 0)
        return 0;
    if (!mm::user_range_ok(buf, count))
        return -EFAULT;

    // Copy through a kernel buffer, one chunk at a time.
    char chunk[SYSCALL_BOUNCE_SIZE];
    uint32_t done = 0;
    while (done < count)
    {
        uint32_t n = count - done;
        if (n > sizeof(chunk))
            n = sizeof(chunk);
        if (mm::copy_from_user(chunk, buf + done, n) < 0)
            return done ? static_cast<SyscallResult>(done) : -EFAULT;
        int w = fs::vfs_write(vf, chunk, n);
        if (w < 0)
            return done ? static_cast<SyscallResult>(done) : w;
        done += static_cast<uint32_t>(w);
        if (static_cast<uint32_t>(w) < n)
            break; // short write (e.g. file size cap)
    }
    return static_cast<SyscallResult>(done);
}

} // namespace syscall
