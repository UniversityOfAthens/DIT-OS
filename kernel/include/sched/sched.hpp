#ifndef KERNEL_SCHED_SCHED_HPP
#define KERNEL_SCHED_SCHED_HPP

#include <stdint.h>
#include "sched/task.hpp"

namespace sched
{

/** Switch from prev to next (saves prev's context, restores next's). Never
 * returns to caller when switching to next. */
extern "C" void switch_to(Task* prev, Task* next);

/** Current running task (set by scheduler) */
extern Task* current_task;

/** Run queue head (runnable tasks) */
extern Task* run_queue_head;

/** Add task to run queue */
void run_queue_add(Task* t);

/** Remove task from run queue; returns new head */
Task* run_queue_remove(Task* t);

/** Pick next task from run queue (round-robin). Returns idle if none. */
Task* schedule_next();

/** Yield: switch to the next runnable task, if any (cooperative). */
void schedule();

/** True if a task other than idle is runnable. Call with IRQs disabled. */
bool has_other_runnable();

/** Tasks blocked until an event (linked through Task::next while blocked). */
struct WaitQueue
{
    Task* head;
};

/** Block the current task on @wq until wake_up_all(@wq). Must be called with
 * interrupts disabled (so the wakeup cannot be missed between checking the
 * condition and sleeping); returns with interrupts still disabled. */
void sleep_on(WaitQueue* wq);

/** Make every task sleeping on @wq runnable. Safe to call from an IRQ. */
void wake_up_all(WaitQueue* wq);

/** Build @t's initial context so switch_to() starts it at @entry on @stack.
 * Returns false if the stack is too small. */
bool task_init(Task* t, void (*entry)(), uint32_t* stack, uint32_t stack_size,
               uint32_t pid);

/** Create a new task with entry_point and stack; add to run queue */
Task* task_create(void (*entry)(), uint32_t* stack, uint32_t stack_size);
Task* task_create_user(uint32_t entry, uint32_t* kernel_stack,
                       uint32_t kernel_stack_size, uint32_t* user_stack,
                       uint32_t user_stack_size);
bool init_user_image();

/** Allocate a new pid for a task (used by task_create) */
uint32_t pid_alloc();

/** Current running task pid (0 if no current task) */
uint32_t current_pid();

/** Terminate the current task and schedule the next one */
void task_exit(int code);

void task_activate(Task* task);

/** Idle task (runs when nothing else runnable) */
extern Task* idle_task;

/** Initialize scheduler: create idle task, run queue */
void sched_init();

/** Start scheduling: leaves the boot stack for good (does not return) */
void sched_start(void (*init_entry)());

/** Idle task setup (called from sched_init) */
void idle_task_init(Task* idle, uint32_t* stack, uint32_t stack_size);

} // namespace sched

#endif
