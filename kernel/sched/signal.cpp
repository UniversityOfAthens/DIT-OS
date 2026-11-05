// DIT-OS Kernel - Signal dispositions, masks, job control and delivery

#include "sched/signal.hpp"
#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "sched/wait.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "mm/uaccess.hpp"
#include "mm/vmm.hpp"
#include "errno_defs.hpp"
#include <stddef.h>
#include <stdint.h>

namespace sched
{

namespace
{

/* sigsuspend() sleepers; woken individually by task_wake() on a signal. */
WaitQueue s_suspend_wait;

constexpr uint32_t flag(int sig)
{
    return sigset_t(1) << (sig - 1);
}

constexpr sigset_t STOP_SIGNALS = flag(SIGSTOP) | flag(SIGTSTP) |
                                  flag(SIGTTIN) | flag(SIGTTOU);

bool valid_signal(int sig)
{
    return sig > 0 && sig < NSIG;
}

/* SIGKILL and SIGSTOP cannot be blocked or caught; silently drop them from a
 * user-supplied mask like POSIX does. */
sigset_t sanitize_mask(sigset_t mask)
{
    return mask & ~(flag(SIGKILL) | flag(SIGSTOP));
}

/* Lowest pending, unblocked signal, or 0. */
int lowest_pending(const Task* t)
{
    sigset_t pending = t->pending_signals & ~t->blocked_signals;
    for (int sig = 1; sig < NSIG; sig++)
    {
        if (pending & flag(sig))
            return sig;
    }
    return 0;
}

void reset_action(SignalAction& action)
{
    action.handler = SIG_DFL;
    action.mask = 0;
    action.flags = 0;
    action.restorer = 0;
}

/* A signal whose generation has no effect can be dropped at once (POSIX
 * allows it), so it neither wakes the target nor makes its syscalls fail. */
bool discard_on_send(const Task* t, int sig)
{
    if (t->blocked_signals & flag(sig))
        return false;
    const SignalAction& action = t->actions[sig];
    if (action.handler == SIG_IGN)
        return true;
    if (action.handler != SIG_DFL)
        return false;
    SignalDefault def = signal_default_action(sig);
    /* SIGCONT's effect (resuming) already happened in signal_send_task. */
    return def == SignalDefault::Ign || def == SignalDefault::Cont;
}

/* EFLAGS a user context may restore: CF PF AF ZF SF DF OF. IF is forced on
 * and IOPL, TF, NT, RF, VM, AC and ID stay clear, so sigreturn can neither
 * disable interrupts on this uniprocessor kernel nor gain I/O privilege. */
constexpr uint32_t USER_EFLAGS_MASK = 0xCD5u;
constexpr uint32_t EFLAGS_FORCED = 0x202u; /* IF | reserved bit 1 */

uint32_t sanitize_eflags(uint32_t eflags)
{
    return (eflags & USER_EFLAGS_MASK) | EFLAGS_FORCED;
}

/* Copy a handler-created frame back into the saved context. */
bool restore_regs(const SignalFrame& frame, uint32_t* regs)
{
    if (frame.eip < mm::VMM_USER_START || frame.eip >= mm::VMM_USER_END)
        return false;
    if (frame.esp < mm::VMM_USER_START || frame.esp > mm::VMM_USER_END)
        return false;

    regs[arch::FRAME_EDI] = frame.edi;
    regs[arch::FRAME_ESI] = frame.esi;
    regs[arch::FRAME_EBP] = frame.ebp;
    regs[arch::FRAME_EBX] = frame.ebx;
    regs[arch::FRAME_EDX] = frame.edx;
    regs[arch::FRAME_ECX] = frame.ecx;
    regs[arch::FRAME_EAX] = frame.eax;
    regs[arch::FRAME_EIP] = frame.eip;
    regs[arch::FRAME_EFLAGS] = sanitize_eflags(frame.eflags);
    regs[arch::FRAME_STACK_ESP] = frame.esp;
    return true;
}

/* Build the delivery block below the interrupted user stack and point the
 * iret frame at the handler. Returns false when the stack cannot hold it. */
bool deliver_handler(Task* t, int sig, const SignalAction& action,
                     uint32_t* regs)
{
    uint32_t user_esp = regs[arch::FRAME_STACK_ESP];
    if (user_esp < mm::VMM_USER_START + sizeof(SignalDelivery) ||
        user_esp > mm::VMM_USER_END)
        return false;
    uint32_t base = user_esp - sizeof(SignalDelivery);

    SignalDelivery delivery;
    uint8_t* raw = reinterpret_cast<uint8_t*>(&delivery);
    for (size_t i = 0; i < sizeof(delivery); i++)
        raw[i] = 0;

    delivery.restorer = action.restorer;
    delivery.signo = static_cast<uint32_t>(sig);
    delivery.frame.magic = SIGNAL_FRAME_MAGIC;
    delivery.frame.signo = static_cast<uint32_t>(sig);
    delivery.frame.eip = regs[arch::FRAME_EIP];
    delivery.frame.eflags = regs[arch::FRAME_EFLAGS];
    delivery.frame.esp = user_esp;
    delivery.frame.edi = regs[arch::FRAME_EDI];
    delivery.frame.esi = regs[arch::FRAME_ESI];
    delivery.frame.ebp = regs[arch::FRAME_EBP];
    delivery.frame.ebx = regs[arch::FRAME_EBX];
    delivery.frame.edx = regs[arch::FRAME_EDX];
    delivery.frame.ecx = regs[arch::FRAME_ECX];
    delivery.frame.eax = regs[arch::FRAME_EAX];
    /* After sigsuspend() the handler must return to the mask the caller had,
     * not the temporary one it slept with. */
    delivery.frame.saved_mask = t->restore_mask ? t->saved_mask :
                                                  t->blocked_signals;

    /* copy_to_user() breaks copy-on-write, so a forked child's frame never
     * lands in a page still shared with its parent. */
    if (mm::copy_to_user(base, &delivery, sizeof(delivery)) < 0)
        return false;

    t->restore_mask = false;
    sigset_t mask = t->blocked_signals | action.mask;
    if (!(action.flags & SA_NODEFER))
        mask |= flag(sig);
    t->blocked_signals = sanitize_mask(mask);
    t->pending_signals &= ~flag(sig);
    if (action.flags & SA_RESETHAND)
        reset_action(t->actions[sig]);

    regs[arch::FRAME_EIP] = action.handler;
    regs[arch::FRAME_STACK_ESP] = base;
    return true;
}

/* The stopping signal's default action on the current task: report it and
 * sleep until continued. Called with IRQs off. */
void stop_current(int sig)
{
    Task* t = current_task;
    t->stop_signal = sig;
    t->job_event = JobEvent::Stopped;
    t->stop_count++;
    signal_notify_parent(t, false);
    task_stop_current();
}

/* Default action of @sig for the current task. Returns only when the task
 * keeps running (ignored, continued, or stopped and then continued). */
void run_default_action(int sig)
{
    switch (signal_default_action(sig))
    {
    case SignalDefault::Term:
    case SignalDefault::Core:
        task_exit(make_wait_status_signal(sig));
        break;
    case SignalDefault::Stop:
        stop_current(sig);
        break;
    case SignalDefault::Ign:
    case SignalDefault::Cont:
        break;
    }
}

/* Is the caller allowed to signal @target? Itself and its descendants. */
bool may_signal(const Task* caller, const Task* target)
{
    return caller && task_is_descendant(target, caller);
}

} // namespace

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

int signal_action_set(int sig, const SignalAction* act, SignalAction* old)
{
    if (!valid_signal(sig))
        return -EINVAL;
    /* SIGKILL and SIGSTOP can be queried but never changed. */
    if ((sig == SIGKILL || sig == SIGSTOP) && act)
        return -EINVAL;
    Task* t = current_task;
    if (!t)
        return -ESRCH;

    if (act)
    {
        if (act->flags & ~SA_FLAG_MASK)
            return -EINVAL;
        if (act->handler != SIG_DFL && act->handler != SIG_IGN)
        {
            /* The handler and restorer run in ring 3: reject kernel or
             * otherwise non-user addresses instead of handing the CPU a
             * wild EIP. */
            if (act->handler < mm::VMM_USER_START ||
                act->handler >= mm::VMM_USER_END)
                return -EINVAL;
            if (act->restorer < mm::VMM_USER_START ||
                act->restorer >= mm::VMM_USER_END)
                return -EINVAL;
        }
    }

    uint32_t flags = arch::irq_save();
    if (old)
        *old = t->actions[sig];
    if (act)
    {
        SignalAction stored = *act;
        if (stored.handler == SIG_DFL || stored.handler == SIG_IGN)
        {
            /* SA_NOCLDSTOP and SA_NOCLDWAIT stay meaningful without a
             * handler; the rest only shape handler delivery. */
            stored.mask = 0;
            stored.flags &= (SA_NOCLDSTOP | SA_NOCLDWAIT);
            stored.restorer = 0;
        }
        else
        {
            stored.mask = sanitize_mask(stored.mask);
        }
        t->actions[sig] = stored;
        /* Setting SIG_IGN (or a default of ignore) discards what is pending,
         * as POSIX requires. */
        if (discard_on_send(t, sig))
            t->pending_signals &= ~flag(sig);
    }
    arch::irq_restore(flags);
    return 0;
}

int signal_mask_set(int how, const sigset_t* set, sigset_t* old)
{
    Task* t = current_task;
    if (!t)
        return -ESRCH;
    /* POSIX: @how only matters when there is a new set. */
    if (set && how != SIG_BLOCK && how != SIG_UNBLOCK && how != SIG_SETMASK)
        return -EINVAL;

    uint32_t flags = arch::irq_save();
    if (old)
        *old = t->blocked_signals;
    if (set)
    {
        sigset_t mask = sanitize_mask(*set);
        if (how == SIG_BLOCK)
            t->blocked_signals |= mask;
        else if (how == SIG_UNBLOCK)
            t->blocked_signals &= ~mask;
        else
            t->blocked_signals = mask;
    }
    arch::irq_restore(flags);
    return 0;
}

sigset_t signal_pending_blocked()
{
    Task* t = current_task;
    if (!t)
        return 0;
    uint32_t flags = arch::irq_save();
    sigset_t set = t->pending_signals & t->blocked_signals;
    arch::irq_restore(flags);
    return set;
}

bool signal_blocked_or_ignored(const Task* t, int sig)
{
    if (!t || !valid_signal(sig))
        return true;
    return (t->blocked_signals & flag(sig)) ||
           t->actions[sig].handler == SIG_IGN;
}

void signal_send_task(Task* target, int sig)
{
    if (!target || !valid_signal(sig))
        return;
    uint32_t flags = arch::irq_save();
    if (target->state == TaskState::Exited)
    {
        arch::irq_restore(flags);
        return;
    }
    /* init only receives signals it has a handler for: a default action
     * would kill or stop the process that reaps orphans and runs the
     * console (Linux protects init the same way). Its own faults still
     * reach it through signal_force_current(). */
    if (target->pid == 1 && target->actions[sig].handler == SIG_DFL)
    {
        arch::irq_restore(flags);
        return;
    }

    if (sig == SIGCONT)
    {
        /* SIGCONT resumes the process even when it is blocked or ignored,
         * and cancels pending stop signals. */
        target->pending_signals &= ~STOP_SIGNALS;
        if (target->state == TaskState::Stopped)
        {
            task_continue(target);
            target->job_event = JobEvent::Continued;
            signal_notify_parent(target, false);
        }
    }
    else if (flag(sig) & STOP_SIGNALS)
    {
        target->pending_signals &= ~flag(SIGCONT);
    }

    if (discard_on_send(target, sig))
    {
        arch::irq_restore(flags);
        return;
    }
    target->pending_signals |= flag(sig);
    /* SIGKILL ends a stopped process too: it must run to die. */
    if (sig == SIGKILL)
        task_continue(target);
    /* A blocked signal cannot act, so it does not interrupt a sleep. */
    if (!(target->blocked_signals & flag(sig)))
        task_wake(target);
    arch::irq_restore(flags);
}

void signal_send_group(uint32_t pgid, int sig)
{
    if (pgid == 0 || !valid_signal(sig))
        return;
    uint32_t flags = arch::irq_save();
    for (Task* t = task_list_head(); t; t = t->all_next)
    {
        if (t->pgid == pgid && t->user_mode && t->state != TaskState::Exited)
            signal_send_task(t, sig);
    }
    arch::irq_restore(flags);
}

int signal_send(int32_t pid, int sig)
{
    if (sig < 0 || sig >= NSIG)
        return -EINVAL;
    /* Broadcast (-1) is not supported; INT32_MIN has no positive group. */
    if (pid == -1 || pid == INT32_MIN)
        return -EINVAL;

    /* Lookup, permission check and delivery are one critical section: an IRQ
     * could otherwise change a target's state between check and send. */
    uint32_t flags = arch::irq_save();
    Task* caller = current_task;
    if (!caller)
    {
        arch::irq_restore(flags);
        return -ESRCH;
    }

    if (pid > 0)
    {
        Task* target = task_find(pid);
        int result = 0;
        if (!target)
            result = -ESRCH;
        else if (!may_signal(caller, target))
            result = -EPERM;
        /* Signal 0 only checks; a zombie answers it but must not have its
         * exit status overwritten by a real signal. */
        else if (sig != 0 && target->state != TaskState::Exited)
            signal_send_task(target, sig);
        arch::irq_restore(flags);
        return result;
    }

    uint32_t pgid = pid == 0 ? caller->pgid : static_cast<uint32_t>(-pid);
    bool found = false;
    bool permitted = false;
    for (Task* t = task_list_head(); t; t = t->all_next)
    {
        if (t->pgid != pgid || t->state == TaskState::Exited)
            continue;
        found = true;
        if (!may_signal(caller, t))
            continue;
        permitted = true;
        if (sig != 0)
            signal_send_task(t, sig);
    }
    arch::irq_restore(flags);
    if (!found)
        return -ESRCH;
    return permitted ? 0 : -EPERM;
}

void signal_force_current(int sig)
{
    Task* t = current_task;
    if (!t || !valid_signal(sig))
        return;
    uint32_t flags = arch::irq_save();
    /* Returning from the fault without acting would re-execute the faulting
     * instruction forever, so a blocked or ignored fault signal reverts to
     * its default (Linux force_sig() semantics). */
    if (t->actions[sig].handler == SIG_IGN || (t->blocked_signals & flag(sig)))
    {
        reset_action(t->actions[sig]);
        t->blocked_signals &= ~flag(sig);
    }
    t->pending_signals |= flag(sig);
    arch::irq_restore(flags);
}

bool signal_interrupt_pending()
{
    Task* t = current_task;
    if (!t || !t->user_mode)
        return false;

    uint32_t flags = arch::irq_save();
    for (;;)
    {
        int sig = lowest_pending(t);
        if (sig == 0)
        {
            arch::irq_restore(flags);
            return false;
        }
        const SignalAction& action = t->actions[sig];
        if (action.handler != SIG_DFL && action.handler != SIG_IGN)
        {
            arch::irq_restore(flags);
            return true;
        }
        t->pending_signals &= ~flag(sig);
        if (action.handler == SIG_DFL)
            run_default_action(sig);
    }
}

bool signal_restart_pending()
{
    Task* t = current_task;
    if (!t)
        return false;
    uint32_t flags = arch::irq_save();
    int sig = lowest_pending(t);
    bool restart = sig != 0 && (t->actions[sig].flags & SA_RESTART);
    arch::irq_restore(flags);
    return restart;
}

bool signal_handle_pending(uint32_t* regs)
{
    Task* t = current_task;
    if (!t || !t->user_mode || !regs)
        return false;

    /* Default actions first; what is left is a caught signal (or nothing). */
    if (!signal_interrupt_pending())
    {
        /* sigsuspend() woke for a signal that was then handled by default:
         * nothing will restore the mask on a handler's return, so do it. */
        if (t->restore_mask)
        {
            t->blocked_signals = t->saved_mask;
            t->restore_mask = false;
        }
        return false;
    }

    uint32_t flags = arch::irq_save();
    int sig = lowest_pending(t);
    SignalAction action = t->actions[sig];
    bool delivered = deliver_handler(t, sig, action, regs);
    arch::irq_restore(flags);
    if (!delivered)
    {
        /* The handler frame could not be written (bad or exhausted user
         * stack); report it as the fault it is instead of silently
         * dropping the signal. */
        task_exit(make_wait_status_signal(SIGSEGV));
    }
    return true;
}

bool signal_sigreturn(uint32_t* regs)
{
    Task* t = current_task;
    if (!t || !t->user_mode || !regs)
        return false;
    /* The restorer drops the signal-number argument, so the frame starts at
     * the user stack pointer of the sigreturn call. */
    uint32_t frame_addr = regs[arch::FRAME_STACK_ESP];
    if (frame_addr < mm::VMM_USER_START || frame_addr >= mm::VMM_USER_END ||
        sizeof(SignalFrame) > mm::VMM_USER_END - frame_addr)
        return false;

    /* One fetch into a kernel copy; everything below validates the copy. */
    SignalFrame frame;
    if (mm::copy_from_user(&frame, frame_addr, sizeof(frame)) < 0)
        return false;
    if (frame.magic != SIGNAL_FRAME_MAGIC)
        return false;
    if (!restore_regs(frame, regs))
        return false;

    uint32_t flags = arch::irq_save();
    t->blocked_signals = sanitize_mask(frame.saved_mask);
    arch::irq_restore(flags);
    return true;
}

int signal_suspend(sigset_t mask)
{
    Task* t = current_task;
    if (!t)
        return -ESRCH;
    uint32_t flags = arch::irq_save();
    t->saved_mask = t->blocked_signals;
    t->restore_mask = true;
    t->blocked_signals = sanitize_mask(mask);
    /* sleep_on() returns false once a caught signal is pending; a true
     * return is a wakeup for a signal that was handled by default (or
     * spurious), so keep waiting. */
    while (sleep_on(&s_suspend_wait))
    {
    }
    arch::irq_restore(flags);
    return -EINTR;
}

void signal_notify_parent(Task* child, bool exited)
{
    if (!child)
        return;
    Task* parent = task_find(static_cast<int32_t>(child->ppid));
    if (parent && parent->state != TaskState::Exited)
    {
        bool quiet = !exited &&
                     (parent->actions[SIGCHLD].flags & SA_NOCLDSTOP) != 0;
        if (!quiet)
            signal_send_task(parent, SIGCHLD);
    }
    /* Parents in waitpid() re-scan their children. */
    wake_up_all(&child_wait_queue());
}

} // namespace sched
