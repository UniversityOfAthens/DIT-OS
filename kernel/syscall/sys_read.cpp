// DIT-OS Kernel - sys_read (any open file: console, VFS file or pipe)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "errno_defs.hpp"
#include "mm/uaccess.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_read_impl(const SyscallArgs* args)
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

    // One chunk per call: a short read is valid, and a console or pipe read
    // must not block again for a second chunk once it has data.
    char chunk[SYSCALL_BOUNCE_SIZE];
    uint32_t want = count < sizeof(chunk) ? count : sizeof(chunk);
    int n = fs::vfs_read(vf, chunk, want);
    if (n <= 0)
        return n;
    if (mm::copy_to_user(buf, chunk, static_cast<uint32_t>(n)) < 0)
        return -EFAULT;
    return n;
}

} // namespace syscall
