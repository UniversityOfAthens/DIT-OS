// DIT-OS Kernel - Idle task

#include "sched/sched.hpp"
#include "core/panic.hpp"
#include <stdint.h>

namespace sched
{

// Runs whenever nothing else is runnable. Interrupts are disabled while the
// run queue is checked, and "sti; hlt" re-enables them only as hlt starts, so
// a wakeup IRQ cannot slip in between the check and the halt.
static void idle_loop()
{
    for (;;)
    {
        __asm__ volatile("cli" : : : "memory");
        if (has_other_runnable())
        {
            __asm__ volatile("sti" : : : "memory");
            schedule();
        }
        else
        {
            __asm__ volatile("sti; hlt" : : : "memory");
        }
    }
}

void idle_task_init(Task* idle, uint32_t* stack, uint32_t stack_size)
{
    if (!task_init(idle, idle_loop, stack, stack_size, 0))
        core::panic("idle_task_init: bad idle stack");
}

} // namespace sched
