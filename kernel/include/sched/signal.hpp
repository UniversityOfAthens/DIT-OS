/** DIT-OS Kernel - Signal numbers and masks (POSIX subset; user/kernel ABI). */
#ifndef KERNEL_SCHED_SIGNAL_HPP
#define KERNEL_SCHED_SIGNAL_HPP

#include <stdint.h>

namespace sched
{

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

struct SignalAction
{
    void (*handler)(int);
    sigset_t mask;
    int flags;
};

inline void sigemptyset(sigset_t* s)
{
    if (s)
        *s = 0;
}

inline void sigfillset(sigset_t* s)
{
    if (s)
        *s = ~sigset_t(0);
}

inline void sigaddset(sigset_t* s, int sig)
{
    if (s && sig > 0 && sig < NSIG)
        *s |= (sigset_t(1) << (sig - 1));
}

inline void sigdelset(sigset_t* s, int sig)
{
    if (s && sig > 0 && sig < NSIG)
        *s &= ~(sigset_t(1) << (sig - 1));
}

inline bool sigismember(sigset_t s, int sig)
{
    if (sig <= 0 || sig >= NSIG)
        return false;
    return (s & (sigset_t(1) << (sig - 1))) != 0;
}

/** Default actions (for internal delivery policy). */
enum class SignalDefault : uint8_t
{
    Term,
    Ign,
    Core,
    Stop,
    Cont,
};

} // namespace sched

#endif
