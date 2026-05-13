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

/** Flag set by timer ISR, checked at safe points to invoke schedule() */
extern volatile uint32_t s_need_resched;

/** Pick next task from run queue (round-robin). Returns idle if none. */
Task* schedule_next();

/** Called every timer tick (and on yield): switch to next task if needed */
void schedule();

/** Check if reschedule is needed and yield if so (call from safe points) */
void maybe_yield();

/** Create a new task with entry_point and stack; add to run queue */
Task* task_create(void (*entry)(), uint32_t* stack, uint32_t stack_size);

/** Allocate a new pid for a task (used by task_create) */
uint32_t pid_alloc();

/** Current running task pid (0 if no current task) */
uint32_t current_pid();

/** Terminate the current task and schedule the next one */
void task_exit(int code);

/** Idle task (runs when nothing else runnable) */
extern Task* idle_task;

/** Initialize scheduler: create idle task, run queue */
void sched_init();

/** Create init task and start scheduler (does not return in current context) */
void sched_start(void (*init_entry)());

/** Idle task setup (called from sched_init) */
void idle_task_init(Task* idle, uint32_t* stack, uint32_t stack_size);

/** Return init task entry point (shell) */
void (*get_init_entry())();

/** Return shell entry (used by get_init_entry) */
void (*get_shell_entry())();

} // namespace sched

#endif
