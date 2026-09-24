/** DIT-OS Kernel - Thread (schedulable unit bound to a process). */
#ifndef KERNEL_SCHED_THREAD_HPP
#define KERNEL_SCHED_THREAD_HPP

#include <stdint.h>
#include "sched/task.hpp"
#include "sched/process.hpp"

namespace sched
{

/**
 * Thread: embeds Task for context switch; ties to Process.
 * Current kernel tasks use standalone Task*; Thread is the multi-threading
 * shape.
 */
struct Thread
{
    Task task;
    /** Thread id (main thread often equals process pid). */
    uint32_t tid;
    Process* process;
    /** Other threads in the same process. */
    Thread* next_in_process;
};

} // namespace sched

#endif
