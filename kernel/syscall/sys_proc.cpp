// DIT-OS Kernel - setpgid, getpgid, setsid, getsid (job control)

#include "syscall/syscall_defs.hpp"
#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_setpgid_impl(const SyscallArgs* args);
extern SyscallResult sys_getpgid_impl(const SyscallArgs* args);
extern SyscallResult sys_setsid_impl(const SyscallArgs* args);
extern SyscallResult sys_getsid_impl(const SyscallArgs* args);

namespace
{

/* Does any live task belong to group @pgid in session @sid? Groups are
 * session-homogeneous (setpgid cannot cross a session boundary), so one
 * member decides for the group. */
bool group_exists_in(uint32_t pgid, uint32_t sid)
{
    for (sched::Task* t = sched::task_list_head(); t; t = t->all_next)
    {
        if (t->pgid == pgid && t->sid == sid &&
            t->state != sched::TaskState::Exited)
            return true;
    }
    return false;
}

} // namespace

SyscallResult sys_setpgid_impl(const SyscallArgs* args)
{
    /* Both are non-negative 32-bit ids; reject anything wider or negative
     * before narrowing. */
    if (args->args[0] > static_cast<uint64_t>(INT32_MAX) ||
        args->args[1] > static_cast<uint64_t>(INT32_MAX))
        return -EINVAL;

    sched::Task* caller = sched::current_task;
    if (!caller)
        return -ESRCH;

    uint32_t flags = arch::irq_save();
    uint32_t pid = static_cast<uint32_t>(args->args[0]);
    sched::Task* target = pid == 0 ? caller : sched::task_find(pid);
    /* POSIX: only the caller itself or one of its children. */
    if (!target || target->state == sched::TaskState::Exited ||
        (target != caller && target->ppid != caller->pid))
    {
        arch::irq_restore(flags);
        return -ESRCH;
    }
    /* A process group lives inside one session (L22): the target must stay
     * in the caller's session, or the move would hand another session a
     * member of this one. */
    if (target->sid != caller->sid)
    {
        arch::irq_restore(flags);
        return -EPERM;
    }
    uint32_t pgid = static_cast<uint32_t>(args->args[1]);
    if (pgid == 0)
        pgid = target->pid;
    /* Either start a group named after the target, or join an existing one;
     * a process cannot invent a group id that belongs to nobody, and the
     * group it joins must live in the caller's session. */
    if (pgid != target->pid && !group_exists_in(pgid, caller->sid))
    {
        arch::irq_restore(flags);
        return -EPERM;
    }
    target->pgid = pgid;
    arch::irq_restore(flags);
    return 0;
}

SyscallResult sys_getpgid_impl(const SyscallArgs* args)
{
    if (args->args[0] > static_cast<uint64_t>(INT32_MAX))
        return -EINVAL;
    sched::Task* caller = sched::current_task;
    if (!caller)
        return -ESRCH;

    uint32_t flags = arch::irq_save();
    uint32_t pid = static_cast<uint32_t>(args->args[0]);
    sched::Task* target = pid == 0 ? caller : sched::task_find(pid);
    SyscallResult result = target ? static_cast<SyscallResult>(target->pgid) :
                                    static_cast<SyscallResult>(-ESRCH);
    arch::irq_restore(flags);
    return result;
}

SyscallResult sys_setsid_impl(const SyscallArgs* args)
{
    (void)args;
    sched::Task* caller = sched::current_task;
    if (!caller)
        return -ESRCH;

    uint32_t flags = arch::irq_save();
    /* POSIX: a process group leader cannot leave its group, since the new
     * session would reuse a group id that already exists. */
    if (caller->pgid == caller->pid)
    {
        arch::irq_restore(flags);
        return -EPERM;
    }
    caller->sid = caller->pid;
    caller->pgid = caller->pid;
    arch::irq_restore(flags);
    return 0;
}

SyscallResult sys_getsid_impl(const SyscallArgs* args)
{
    if (args->args[0] > static_cast<uint64_t>(INT32_MAX))
        return -EINVAL;
    sched::Task* caller = sched::current_task;
    if (!caller)
        return -ESRCH;

    uint32_t flags = arch::irq_save();
    uint32_t pid = static_cast<uint32_t>(args->args[0]);
    sched::Task* target = pid == 0 ? caller : sched::task_find(pid);
    /* Like Linux, any live pid may be queried, not only same-session ones. */
    SyscallResult result = target ? static_cast<SyscallResult>(target->sid) :
                                    static_cast<SyscallResult>(-ESRCH);
    arch::irq_restore(flags);
    return result;
}

} // namespace syscall
