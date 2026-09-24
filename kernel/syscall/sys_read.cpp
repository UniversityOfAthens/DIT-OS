// DIT-OS Kernel - sys_read (fd 0 -> console TTY, fds >= 3 -> VFS)

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/fd.hpp"
#include "fs/vfs.hpp"
#include "syscall/tty.hpp"
#include "errno_defs.hpp"
#include "mm/uaccess.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_read_impl(const SyscallArgs* args)
{
    int fd = static_cast<int>(args->args[0]);
    uintptr_t buf = static_cast<uintptr_t>(args->args[1]);
    uint32_t count = static_cast<uint32_t>(args->args[2]);
    if (count == 0)
        return 0;
    if (!mm::user_range_ok(buf, count))
        return -EFAULT;

    fs::file* vf = nullptr;
    if (fd != 0)
    {
        vf = fd_get(fd);
        if (!vf)
            return -EBADF;
    }

    // One chunk per call: a short read is valid, and a console read must not
    // block again for a second chunk once it has data.
    char chunk[SYSCALL_BOUNCE_SIZE];
    uint32_t want = count < sizeof(chunk) ? count : sizeof(chunk);
    int n = vf ? fs::vfs_read(vf, chunk, want) : tty_read(chunk, want);
    if (n <= 0)
        return n;
    if (mm::copy_to_user(buf, chunk, static_cast<uint32_t>(n)) < 0)
        return -EFAULT;
    return n;
}

} // namespace syscall
