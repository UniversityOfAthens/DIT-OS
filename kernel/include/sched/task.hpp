#ifndef KERNEL_SCHED_TASK_HPP
#define KERNEL_SCHED_TASK_HPP

#include <stdint.h>

namespace sched
{

/** Saved context for context switch (order must match context.asm) */
struct Context {
    uint32_t edi, esi, ebp, ebx, esp, eip;
};

enum class TaskState : uint8_t {
    Runnable,
    Running,
    Blocked,
    Exited,
};

struct Task {
    Context ctx;
    TaskState state;
    Task* next; /* run queue link */
    uint32_t stack_top; /* kernel stack top (for cleanup) */
    uint32_t stack_sz;
    uint32_t pid; /* process/thread id (monotonic for now) */
};

} // namespace sched

#endif
