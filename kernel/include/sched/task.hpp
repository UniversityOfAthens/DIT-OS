#ifndef KERNEL_SCHED_TASK_HPP
#define KERNEL_SCHED_TASK_HPP

#include <stdint.h>

namespace sched
{

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
    Task* next; /* run queue link */
    uint32_t stack_top;
    uint32_t stack_sz;
    uint32_t pid;
    uint32_t page_directory;
    uint32_t user_stack_top;
    bool user_mode;
};

} // namespace sched

#endif
