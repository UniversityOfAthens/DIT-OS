// DIT-OS Kernel - Signal default actions and cooperative delivery

#include "sched/signal.hpp"
#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "sched/wait.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace sched
{

SignalDefault signal_default_action(int sig)
{
    switch (sig)
    {
    case SIGCHLD:
    case SIGURG:
    case SIGWINCH:
        return SignalDefault::Ign;
    case SIGCONT:
        return SignalDefault::Cont;
    case SIGSTOP:
    case SIGTSTP:
    case SIGTTIN:
    case SIGTTOU:
        return SignalDefault::Stop;
    case SIGQUIT:
    case SIGILL:
    case SIGTRAP:
    case SIGABRT:
    case SIGBUS:
    case SIGFPE:
    case SIGSEGV:
    case SIGXCPU:
    case SIGXFSZ:
    case SIGSYS:
        return SignalDefault::Core;
    default:
        return SignalDefault::Term;
    }
}

/* Lowest pending signal that terminates the process, or 0 if none. */
static int pending_fatal(const Task* t)
{
    for (int sig = 1; sig < NSIG; sig++)
    {
        if (!sigismember(t->pending_signals, sig))
            continue;
        SignalDefault action = signal_default_action(sig);
        if (action == SignalDefault::Term || action == SignalDefault::Core)
            return sig;
    }
    return 0;
}

int signal_send_pid(int32_t pid, int sig)
{
    if (sig < 0 || sig >= NSIG)
        return -EINVAL;
    if (pid <= 0)
        return -EINVAL;

    /* Lookup, permission check and wakeup must be one critical section: an
     * IRQ could otherwise wake the target between the state check and the
     * wait-queue removal. */
    uint32_t flags = arch::irq_save();
    Task* target = task_find(pid);
    if (!target)
    {
        arch::irq_restore(flags);
        return -ESRCH;
    }

    Task* caller = current_task;
    if (target != caller && (!caller || target->ppid != caller->pid))
    {
        arch::irq_restore(flags);
        return -EPERM;
    }

    /* Signal 0 only checks that the target exists and is ours to signal. */
    if (sig == 0)
    {
        arch::irq_restore(flags);
        return 0;
    }
    if (target->state == TaskState::Exited)
    {
        /* A zombie answers existence checks, but a fatal signal must not
         * overwrite the status its parent is waiting for. */
        arch::irq_restore(flags);
        return 0;
    }

    SignalDefault action = signal_default_action(sig);
    if (action == SignalDefault::Stop)
    {
        /* Stopped tasks and SIGCONT need job control, which does not exist
         * yet. Report it instead of silently ignoring the request. */
        arch::irq_restore(flags);
        return -ENOSYS;
    }
    if (action == SignalDefault::Term || action == SignalDefault::Core)
    {
        sigaddset(&target->pending_signals, sig);
        task_wake(target);
    }
    arch::irq_restore(flags);
    return 0;
}

void signal_check_current()
{
    Task* t = current_task;
    if (!t)
        return;
    int sig = pending_fatal(t);
    if (sig == 0)
        return;
    task_exit(make_wait_status_signal(sig));
}

} // namespace sched
