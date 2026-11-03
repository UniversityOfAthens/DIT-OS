/** DIT-OS Kernel - Signal numbers and masks (POSIX subset; user/kernel ABI). */
#ifndef KERNEL_SCHED_SIGNAL_HPP
#define KERNEL_SCHED_SIGNAL_HPP

#include <stdint.h>

/* cppcheck's unix32 platform predefines some POSIX signal names as numbers,
 * which would expand inside the enum below. The kernel is freestanding and
 * has no system <signal.h>; these enum values are the canonical DIT-OS ABI. */
#ifdef SIGINT
#undef SIGINT
#endif
#ifdef SIGABRT
#undef SIGABRT
#endif
#ifdef SIGFPE
#undef SIGFPE
#endif
#ifdef SIGKILL
#undef SIGKILL
#endif
#ifdef SIGSEGV
#undef SIGSEGV
#endif
#ifdef SIGTERM
#undef SIGTERM
#endif

namespace sched
{

struct Task;

/** POSIX-style signal numbers (1-based; bit (sig-1) in sigset_t). */
enum Signal : int
{
    SIGHUP = 1,
    SIGINT = 2,
    SIGQUIT = 3,
    SIGILL = 4,
    SIGTRAP = 5,
    SIGABRT = 6,
    SIGBUS = 7,
    SIGFPE = 8,
    SIGKILL = 9,
    SIGUSR1 = 10,
    SIGSEGV = 11,
    SIGUSR2 = 12,
    SIGPIPE = 13,
    SIGALRM = 14,
    SIGTERM = 15,
    SIGSTKFLT = 16,
    SIGCHLD = 17,
    SIGCONT = 18,
    SIGSTOP = 19,
    SIGTSTP = 20,
    SIGTTIN = 21,
    SIGTTOU = 22,
    SIGURG = 23,
    SIGXCPU = 24,
    SIGXFSZ = 25,
    SIGVTALRM = 26,
    SIGPROF = 27,
    SIGWINCH = 28,
    SIGIO = 29,
    SIGPWR = 30,
    SIGSYS = 31,
    NSIG = 32,
};

/** Bitmask for signals 1..31 (bit i corresponds to signal i+1). */
using sigset_t = uint32_t;

/** Dispositions stored in SignalAction::handler. A real handler is a user
 *  address, so these two reserved values cannot collide with one. */
constexpr uint32_t SIG_DFL = 0;
constexpr uint32_t SIG_IGN = 1;

/* sigaction flags (Linux values). Only the ones the kernel implements are
 * accepted; anything else is -EINVAL. */
constexpr uint32_t SA_NOCLDSTOP = 1u << 0;
constexpr uint32_t SA_RESTART = 1u << 28;
constexpr uint32_t SA_NODEFER = 1u << 30;
constexpr uint32_t SA_RESETHAND = 1u << 31;
constexpr uint32_t SA_FLAG_MASK = SA_NOCLDSTOP | SA_RESTART | SA_NODEFER |
                                  SA_RESETHAND;

/* sigprocmask() operations. */
constexpr int SIG_BLOCK = 0;
constexpr int SIG_UNBLOCK = 1;
constexpr int SIG_SETMASK = 2;

/** User/kernel ABI layout shared with userland/include/signal.h. The restorer
 *  is a user trampoline that executes SYS_sigreturn. */
struct SignalAction
{
    uint32_t handler;
    sigset_t mask;
    uint32_t flags;
    uint32_t restorer;
};

/** Magic at the start of a signal frame; SYS_sigreturn rejects anything else
 *  so a stray call cannot restore an attacker-chosen context. */
constexpr uint32_t SIGNAL_FRAME_MAGIC = 0x51A1F00Du;

/** Saved user context written below the interrupted stack by delivery. The
 *  restorer skips the two words before it, so SYS_sigreturn receives a
 *  pointer to this struct in the saved user ESP. */
struct SignalFrame
{
    uint32_t magic;
    uint32_t signo;
    uint32_t eip;
    uint32_t eflags;
    uint32_t esp;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    sigset_t saved_mask;
};

/** What delivery places on the user stack: [restorer][signo][frame].
 *  The handler starts at the restorer word; its `ret` jumps to the restorer
 *  with ESP at signo, the restorer drops it, and SYS_sigreturn then sees the
 *  frame pointer. */
struct SignalDelivery
{
    uint32_t restorer;
    uint32_t signo;
    SignalFrame frame;
};

/** Default actions (for internal delivery policy). */
enum class SignalDefault : uint8_t
{
    Term,
    Ign,
    Core,
    Stop,
    Cont,
};

/** Default action for @sig (POSIX subset). @sig must be in 1..NSIG-1. */
SignalDefault signal_default_action(int sig);

/** Send @sig to process @pid (> 0), to the caller's process group (0) or to
 *  group -@pid (< -1). The caller may signal itself and its descendants;
 *  signal 0 only checks that. Returns 0, or -EINVAL (bad pid or signal,
 *  including the unsupported broadcast -1), -ESRCH (no such process or
 *  group), -EPERM (no permitted target). */
int signal_send(int32_t pid, int sig);

/** Kernel-generated signal (no permission check), e.g. SIGCHLD, SIGPIPE,
 *  SIGTTIN. Safe from IRQ context. */
void signal_send_task(Task* target, int sig);

/** Kernel-generated signal to every live member of process group @pgid
 *  (TTY ^C/^Z). Safe from IRQ context. */
void signal_send_group(uint32_t pgid, int sig);

/** Synchronous fault (#PF, #UD, ...) in the current task. A fault cannot be
 *  blocked or ignored: such a disposition is reset to the default first, so
 *  the task is either sent to its handler or terminated. */
void signal_force_current(int sig);

/** True when @sig would be discarded or held for @t: it is blocked, or its
 *  disposition is SIG_IGN (job-control checks such as SIGTTOU). */
bool signal_blocked_or_ignored(const Task* t, int sig);

/** Install/query @sig's disposition. @act and @old may be null. Returns 0 or
 *  -EINVAL (bad signal or flags, a handler outside user space, or an attempt
 *  to change SIGKILL/SIGSTOP). */
int signal_action_set(int sig, const SignalAction* act, SignalAction* old);

/** Change the current task's blocked mask. Returns 0, or -EINVAL for an
 *  unknown @how with a non-null @set. SIGKILL/SIGSTOP are silently dropped
 *  from the mask. */
int signal_mask_set(int how, const sigset_t* set, sigset_t* old);

/** Signals pending and blocked for the current task (for sigpending()). */
sigset_t signal_pending_blocked();

/** Run the default actions of the current task's pending, unblocked signals
 *  (terminate, stop until SIGCONT, discard). Returns true when a caught
 *  signal is still pending: a blocking syscall then returns -EINTR so the
 *  handler can run on the way out. IRQs must be disabled by the caller or
 *  this disables them itself. */
bool signal_interrupt_pending();

/** True when the signal that will be delivered next has SA_RESTART, so an
 *  interrupted syscall should be restarted instead of returning -EINTR. */
bool signal_restart_pending();

/** On return to user mode: process pending signals and enter at most one
 *  handler by rewriting the saved frame @regs. Returns true when a handler
 *  was entered. */
bool signal_handle_pending(uint32_t* regs);

/** SYS_sigreturn: restore the context saved by delivery from the frame at
 *  the user stack pointer in @regs. Returns false for an invalid frame. */
bool signal_sigreturn(uint32_t* regs);

/** SYS_sigsuspend: replace the blocked mask with @mask and sleep until a
 *  caught signal is pending. The old mask comes back when the handler
 *  returns. Always returns -EINTR (a terminating signal never returns). */
int signal_suspend(sigset_t mask);

/** Child state change: record it for waitpid() and notify the parent with
 *  SIGCHLD (stop/continue only without SA_NOCLDSTOP). IRQs must be off. */
void signal_notify_parent(Task* child, bool exited);

} // namespace sched

#endif
