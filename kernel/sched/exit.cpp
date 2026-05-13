// DIT-OS Kernel - Task exit (minimal: mark exited + schedule next)

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include <stdint.h>

namespace sched
{

void task_exit(int code)
{
    (void)code;
    if (!current_task)
        for (;;)
            asm volatile("hlt");

    Task* prev = current_task;
    prev->state = TaskState::Exited;

    /* Remove the exiting task from the run queue so it won't be scheduled
     * again. */
    run_queue_remove(prev);

    /* Switch to the next runnable task. Never returns for the exiting task. */
    Task* next = schedule_next();
    if (!next)
        next = idle_task;
    current_task = next;
    switch_to(prev, next);

    /* Should be unreachable; keep compiler happy. */
    for (;;)
        asm volatile("hlt");
}

} // namespace sched
