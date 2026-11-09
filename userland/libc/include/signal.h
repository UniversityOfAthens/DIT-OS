#ifndef USERLAND_LIBC_SIGNAL_H
#define USERLAND_LIBC_SIGNAL_H

/* POSIX signals for DIT-OS userland. Numbers, flags and struct sigaction
 * match kernel/include/sched/signal.hpp (the kernel ABI). Wrappers return
 * 0 or the raw negative errno; failures also set errno. */

#include "errno.h"
#include "syscall.h"

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPWR 30
#define SIGSYS 31
#define NSIG 32

typedef unsigned int sigset_t;
typedef void (*sighandler_t)(int);

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t) - 1)

#define SA_NOCLDSTOP 0x00000001u
#define SA_NOCLDWAIT 0x00000002u
#define SA_RESTART 0x10000000u
#define SA_NODEFER 0x40000000u
#define SA_RESETHAND 0x80000000u

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

/* Same layout as the kernel's SignalAction (four 32-bit words). The
 * restorer is filled in by sigaction() below. */
struct sigaction
{
    sighandler_t sa_handler;
    sigset_t sa_mask;
    unsigned int sa_flags;
    void (*sa_restorer)(void);
};

/* crt0.asm: returns from a handler through SYS_sigreturn. */
void __dit_sig_restorer(void);

static inline int dit_sig_valid(int sig)
{
    return sig > 0 && sig < NSIG;
}

static inline int sigemptyset(sigset_t* set)
{
    *set = 0;
    return 0;
}

static inline int sigfillset(sigset_t* set)
{
    *set = ~0u;
    return 0;
}

static inline int sigaddset(sigset_t* set, int sig)
{
    if (!dit_sig_valid(sig))
        return -EINVAL;
    *set |= 1u << (sig - 1);
    return 0;
}

static inline int sigdelset(sigset_t* set, int sig)
{
    if (!dit_sig_valid(sig))
        return -EINVAL;
    *set &= ~(1u << (sig - 1));
    return 0;
}

static inline int sigismember(const sigset_t* set, int sig)
{
    if (!dit_sig_valid(sig))
        return -EINVAL;
    return (*set >> (sig - 1)) & 1u;
}

static inline int sigaction(int sig, const struct sigaction* act,
                            struct sigaction* old)
{
    struct sigaction k;
    if (act)
    {
        k = *act;
        k.sa_restorer = __dit_sig_restorer;
    }
    return dit_syscall(SYS_sigaction, (unsigned int)sig,
                       (unsigned int)(act ? &k : 0), (unsigned int)old, 0, 0);
}

/* Install @handler with no extra mask, restarting interrupted syscalls
 * (BSD/glibc signal() semantics). Returns the old handler or SIG_ERR. */
static inline sighandler_t signal(int sig, sighandler_t handler)
{
    struct sigaction act;
    struct sigaction old;
    act.sa_handler = handler;
    act.sa_mask = 0;
    act.sa_flags = SA_RESTART;
    act.sa_restorer = 0;
    if (sigaction(sig, &act, &old) < 0)
        return SIG_ERR;
    return old.sa_handler;
}

static inline int sigprocmask(int how, const sigset_t* set, sigset_t* old)
{
    return dit_syscall(SYS_sigprocmask, (unsigned int)how, (unsigned int)set,
                       (unsigned int)old, 0, 0);
}

static inline int sigpending(sigset_t* set)
{
    return dit_syscall(SYS_sigpending, (unsigned int)set, 0, 0, 0, 0);
}

/* Sleep with *@mask blocked until a handler runs; always -EINTR. */
static inline int sigsuspend(const sigset_t* mask)
{
    return dit_syscall(SYS_sigsuspend, *mask, 0, 0, 0, 0);
}

static inline int raise(int sig)
{
    return kill(getpid(), sig);
}

/* Sleep until a signal handler runs; always -EINTR. */
static inline int pause(void)
{
    sigset_t mask;
    int r = sigprocmask(SIG_BLOCK, 0, &mask);
    if (r < 0)
        return r;
    return sigsuspend(&mask);
}

#endif
