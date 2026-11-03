/** DIT-OS Kernel - waitpid status encoding and helpers (Linux-compatible
 * layout). */
#ifndef KERNEL_SCHED_WAIT_HPP
#define KERNEL_SCHED_WAIT_HPP

#include <stdint.h>

namespace sched
{

/** waitpid(2) option bits (POSIX subset). */
constexpr int WNOHANG = 1;
constexpr int WUNTRACED = 2;
constexpr int WCONTINUED = 4;

/** Decode status returned from wait / waitpid (see make_wait_status_*). */
inline bool wifexited(int status)
{
    return (status & 0xff) == 0;
}

inline int wexitstatus(int status)
{
    return (status >> 8) & 0xff;
}

inline bool wifsignaled(int status)
{
    const int lo = status & 0x7f;
    return lo != 0 && lo != 0x7f;
}

inline int wtermsig(int status)
{
    return status & 0x7f;
}

inline bool wifstopped(int status)
{
    return (status & 0xff) == 0x7f;
}

inline int wstopsig(int status)
{
    return (status >> 8) & 0xff;
}

/** Continued after SIGCONT (Linux uses full 16-bit 0xffff). */
inline bool wifcontinued(int status)
{
    return (status & 0xffff) == 0xffff;
}

/** Status reported for a continued child (WCONTINUED). */
constexpr int WAIT_STATUS_CONTINUED = 0xffff;

/** Build status word for a normal exit(code), code 0..255. */
inline int make_wait_status_exit(int code8)
{
    return (code8 & 0xff) << 8;
}

/** Build status for stopped-by-signal. */
inline int make_wait_status_stop(int sig)
{
    return (sig << 8) | 0x7f;
}

/** Build status for terminated by signal (no core dump bit set here). */
inline int make_wait_status_signal(int sig)
{
    return sig & 0x7f;
}

} // namespace sched

#endif
