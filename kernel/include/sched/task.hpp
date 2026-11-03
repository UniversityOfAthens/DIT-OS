#ifndef KERNEL_SCHED_TASK_HPP
#define KERNEL_SCHED_TASK_HPP

#include <stdint.h>
#include "sched/signal.hpp"

namespace sched
{

struct WaitQueue;

/** Saved context for context switch (order must match context.asm) */
struct Context
{
    uint32_t edi, esi, ebp, ebx, esp, eip;
};

enum class TaskState : uint8_t
{
    Runnable,
    Running,
    Blocked,
    Stopped, /* job control: off the run queue until SIGCONT or SIGKILL */
    Exited,
};

/** Child state change not yet collected by waitpid(). */
enum class JobEvent : uint8_t
{
    None,
    Stopped, /* reported with WUNTRACED */
    Continued, /* reported with WCONTINUED */
};

/** Longest working directory, including the NUL. */
constexpr uint32_t TASK_CWD_MAX = 128;

/** Written to the lowest word of every task stack; schedule() panics if it is
 * overwritten, which means the task overflowed its stack. */
constexpr uint32_t STACK_CANARY = 0x57AC6CA7;

struct Task
{
    Context ctx;
    TaskState state;
    Task* next; /* run queue or wait queue link */
    Task* all_next; /* every live task, for parent/child bookkeeping */
    uint32_t stack_top;
    uint32_t stack_sz;
    uint32_t pid;
    uint32_t page_directory;
    uint32_t user_stack_top;
    bool user_mode;
    /* Process state: set when the task is reaped by its parent. */
    uint32_t ppid;
    int32_t exit_code;
    int fd_table; /* index into the per-process fd table pool, -1 if none */
    /** Signals pending delivery (bit set) and the current blocked mask. */
    sigset_t pending_signals;
    sigset_t blocked_signals;
    /** Per-signal dispositions; index with the signal number. */
    SignalAction actions[NSIG];
    /** sigsuspend(): the mask to put back once a handler has been entered
     *  (valid while restore_mask is set). */
    sigset_t saved_mask;
    bool restore_mask;
    /** Process group (job control). Inherited across fork(). */
    uint32_t pgid;
    /** Unreported stop/continue for waitpid(), and the stopping signal. */
    JobEvent job_event;
    int stop_signal;
    /** Times the task has stopped; lets a retry loop tell "stopped and
     *  continued" from "the stop signal was discarded". */
    uint32_t stop_count;
    /** Absolute, normalised working directory (inherited across fork). */
    char cwd[TASK_CWD_MAX];
    /** Wait queue the task is blocked on, or nullptr while runnable. */
    WaitQueue* wait_queue;
};

} // namespace sched

#endif
