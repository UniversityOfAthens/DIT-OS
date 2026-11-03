// DIT-OS Kernel - sigaction, sigprocmask, sigpending, sigsuspend

#include "syscall/syscall_defs.hpp"
#include "sched/signal.hpp"
#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_sigaction_impl(const SyscallArgs* args);
extern SyscallResult sys_sigprocmask_impl(const SyscallArgs* args);
extern SyscallResult sys_sigpending_impl(const SyscallArgs* args);
extern SyscallResult sys_sigsuspend_impl(const SyscallArgs* args);

SyscallResult sys_sigaction_impl(const SyscallArgs* args)
{
    /* Bound while still 64-bit so a huge value cannot truncate into range. */
    if (args->args[0] >= static_cast<uint64_t>(sched::NSIG))
        return -EINVAL;
    int sig = static_cast<int>(args->args[0]);
    uintptr_t user_act = static_cast<uintptr_t>(args->args[1]);
    uintptr_t user_old = static_cast<uintptr_t>(args->args[2]);

    /* Fetch the new action once; validation runs on the kernel copy. */
    sched::SignalAction act;
    if (user_act && mm::copy_from_user(&act, user_act, sizeof(act)) < 0)
        return -EFAULT;
    /* Check the output before changing anything, so a bad pointer does not
     * leave a half-done call behind. */
    if (user_old && !mm::user_range_ok(user_old, sizeof(sched::SignalAction)))
        return -EFAULT;

    sched::SignalAction old;
    old.handler = 0;
    old.mask = 0;
    old.flags = 0;
    old.restorer = 0;
    int r = sched::signal_action_set(sig, user_act ? &act : nullptr,
                                     user_old ? &old : nullptr);
    if (r < 0)
        return r;
    if (user_old && mm::copy_to_user(user_old, &old, sizeof(old)) < 0)
        return -EFAULT;
    return 0;
}

SyscallResult sys_sigprocmask_impl(const SyscallArgs* args)
{
    uintptr_t user_set = static_cast<uintptr_t>(args->args[1]);
    /* @how only matters with a new set (POSIX); bound it while 64-bit. */
    if (user_set && args->args[0] > static_cast<uint64_t>(sched::SIG_SETMASK))
        return -EINVAL;
    int how = static_cast<int>(args->args[0] & 0x3u);
    uintptr_t user_old = static_cast<uintptr_t>(args->args[2]);

    sched::sigset_t set = 0;
    if (user_set && mm::copy_from_user(&set, user_set, sizeof(set)) < 0)
        return -EFAULT;
    if (user_old && !mm::user_range_ok(user_old, sizeof(sched::sigset_t)))
        return -EFAULT;

    sched::sigset_t old = 0;
    int r = sched::signal_mask_set(how, user_set ? &set : nullptr,
                                   user_old ? &old : nullptr);
    if (r < 0)
        return r;
    if (user_old && mm::copy_to_user(user_old, &old, sizeof(old)) < 0)
        return -EFAULT;
    return 0;
}

SyscallResult sys_sigpending_impl(const SyscallArgs* args)
{
    uintptr_t user_set = static_cast<uintptr_t>(args->args[0]);
    sched::sigset_t set = sched::signal_pending_blocked();
    if (mm::copy_to_user(user_set, &set, sizeof(set)) < 0)
        return -EFAULT;
    return 0;
}

SyscallResult sys_sigsuspend_impl(const SyscallArgs* args)
{
    /* The mask is passed by value (one register), like Linux's original
     * sigsuspend; there is no user pointer to fetch. */
    return sched::signal_suspend(static_cast<sched::sigset_t>(args->args[0]));
}

} // namespace syscall
