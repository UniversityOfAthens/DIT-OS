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
    Exited,
};

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
    /** Signals whose default action is Term/Core, waiting to be delivered at
     *  the next syscall exit or before the task blocks. */
    sigset_t pending_signals;
    /** Wait queue the task is blocked on, or nullptr while runnable. */
    WaitQueue* wait_queue;
};

} // namespace sched

#endif
