// DIT-OS Kernel - Task exit: release the process' resources and switch away

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "core/panic.hpp"
#include "mm/vmm.hpp"
#include "syscall/fd.hpp"
#include <stdint.h>

namespace sched
{

void task_exit(int status)
{
    if (!current_task)
        for (;;)
            asm volatile("hlt");

    // IRQs stay off: the run queue is shared with wake_up_all(), and this
    // context is never resumed to restore them.
    arch::irq_save();
    Task* prev = current_task;
    /* PID 1 is the reaper; if it exits there is no process left to adopt
     * orphans, so fail loudly instead of idling forever (M36). */
    if (prev->pid == 1)
    {
        core::panic("init (PID 1) exited; no process left to reap orphans");
        for (;;)
            asm volatile("hlt");
    }
    /* Children keep their parent's pid in ppid; adopt them before this pid
     * becomes a zombie that nobody will ever reap (M40). */
    task_reparent_children(prev->pid, 1);
    /* @status is already encoded for waitpid() (sched/wait.hpp). A task
     * calling task_exit() is running, so it is not on a wait queue. */
    prev->exit_code = status;
    prev->state = TaskState::Exited;

    /* Remove the exiting task from the run queue so it won't be scheduled
     * again. It stays in the task list until its parent reaps it, because
     * waitpid() still needs its pid and exit code. */
    run_queue_remove(prev);

    if (prev->fd_table >= 0)
    {
        syscall::fd_table_release(prev->fd_table);
        prev->fd_table = -1;
    }
    if (prev->page_directory)
    {
        mm::vmm_destroy_user_directory(prev->page_directory);
        prev->page_directory = 0;
    }
    /* The kernel stack is freed by the parent in wait_child(): switch_to()
     * is still using it. */

    wake_up_all(&child_wait_queue());

    /* Switch to the next runnable task. Never returns for the exiting task. */
    Task* next = schedule_next();
    if (!next)
        next = idle_task;
    task_activate(next);
    switch_to(prev, next);

    /* Should be unreachable; keep compiler happy. */
    for (;;)
        asm volatile("hlt");
}

} // namespace sched
