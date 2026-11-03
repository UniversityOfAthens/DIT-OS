// DIT-OS Kernel - chdir, getcwd

#include "syscall/syscall_defs.hpp"
#include "syscall/path.hpp"
#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "fs/vfs.hpp"
#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_chdir_impl(const SyscallArgs* args);
extern SyscallResult sys_getcwd_impl(const SyscallArgs* args);

SyscallResult sys_chdir_impl(const SyscallArgs* args)
{
    sched::Task* t = sched::current_task;
    if (!t)
        return -ESRCH;
    char path[sched::TASK_CWD_MAX];
    int len = path_from_user(path, sizeof(path),
                             static_cast<uintptr_t>(args->args[0]));
    if (len < 0)
        return len;

    fs::stat st;
    int r = fs::vfs_stat(path, &st);
    if (r < 0)
        return r;
    if ((st.st_mode & S_IFMT) != S_IFDIR)
        return -ENOTDIR;

    /* path_normalize() bounded it by TASK_CWD_MAX, NUL included. */
    for (int i = 0; i <= len; i++)
        t->cwd[i] = path[i];
    return 0;
}

SyscallResult sys_getcwd_impl(const SyscallArgs* args)
{
    sched::Task* t = sched::current_task;
    if (!t)
        return -ESRCH;
    uintptr_t buf = static_cast<uintptr_t>(args->args[0]);
    uint64_t size = args->args[1];

    uint32_t len = 0;
    while (len < sched::TASK_CWD_MAX - 1 && t->cwd[len])
        len++;
    /* Linux convention: the result includes the NUL, -ERANGE if it does
     * not fit. */
    if (size < static_cast<uint64_t>(len) + 1)
        return -ERANGE;
    char copy[sched::TASK_CWD_MAX];
    for (uint32_t i = 0; i < len; i++)
        copy[i] = t->cwd[i];
    copy[len] = '\0';
    if (mm::copy_to_user(buf, copy, len + 1) < 0)
        return -EFAULT;
    return static_cast<SyscallResult>(len + 1);
}

} // namespace syscall
