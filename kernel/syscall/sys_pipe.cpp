// DIT-OS Kernel - pipe, dup, dup2

#include "syscall/syscall_defs.hpp"
#include "syscall/fd.hpp"
#include "fs/pipe.hpp"
#include "fs/vfs.hpp"
#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_pipe_impl(const SyscallArgs* args);
extern SyscallResult sys_dup_impl(const SyscallArgs* args);
extern SyscallResult sys_dup2_impl(const SyscallArgs* args);

namespace
{

/* fds are small non-negative ints; bound them while still 64-bit so a value
 * such as 0x1_0000_0001 cannot truncate into a valid descriptor. */
bool fd_arg_ok(uint64_t value)
{
    return value < static_cast<uint64_t>(FD_TABLE_SIZE);
}

} // namespace

SyscallResult sys_pipe_impl(const SyscallArgs* args)
{
    uintptr_t user_fds = static_cast<uintptr_t>(args->args[0]);
    int32_t fds[2];
    /* Fail before creating anything the caller could never learn about. */
    if (!mm::user_range_ok(user_fds, sizeof(fds)))
        return -EFAULT;

    fs::file* read_end = nullptr;
    fs::file* write_end = nullptr;
    int r = fs::pipe_create(&read_end, &write_end);
    if (r < 0)
        return r;

    fds[0] = fd_install(read_end);
    if (fds[0] < 0)
    {
        fs::vfs_close(read_end);
        fs::vfs_close(write_end);
        return fds[0];
    }
    fds[1] = fd_install(write_end);
    if (fds[1] < 0)
    {
        fd_close(fds[0]);
        fs::vfs_close(write_end);
        return fds[1];
    }
    if (mm::copy_to_user(user_fds, fds, sizeof(fds)) < 0)
    {
        fd_close(fds[0]);
        fd_close(fds[1]);
        return -EFAULT;
    }
    return 0;
}

SyscallResult sys_dup_impl(const SyscallArgs* args)
{
    if (!fd_arg_ok(args->args[0]))
        return -EBADF;
    return fd_dup(static_cast<int>(args->args[0]));
}

SyscallResult sys_dup2_impl(const SyscallArgs* args)
{
    if (!fd_arg_ok(args->args[0]) || !fd_arg_ok(args->args[1]))
        return -EBADF;
    return fd_dup2(static_cast<int>(args->args[0]),
                   static_cast<int>(args->args[1]));
}

} // namespace syscall
