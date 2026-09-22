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

/** Written to the lowest word of every task stack; schedule() panics if it is
 * overwritten, which means the task overflowed its stack. */
constexpr uint32_t STACK_CANARY = 0x57AC6CA7;

struct Task {
    Context ctx;
    TaskState state;
    Task* next; /* run queue link */
    uint32_t stack_top; /* lowest address of the kernel stack */
    uint32_t stack_sz;
    uint32_t pid; /* process/thread id (monotonic for now) */
};

} // namespace sched

#endif
