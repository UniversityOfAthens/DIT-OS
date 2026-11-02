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
 * condition and sleeping); returns with interrupts still disabled. A pending
 * fatal signal terminates the task instead of blocking. */
void sleep_on(WaitQueue* wq);

/** Make every task sleeping on @wq runnable. Safe to call from an IRQ. */
void wake_up_all(WaitQueue* wq);

/** Find a task by pid (zombies included), or nullptr. */
Task* task_find(int32_t pid);

/** Remove @t from its wait queue and make it runnable. Does nothing unless
 *  @t is blocked. Must be called with interrupts disabled. */
void task_wake(Task* t);

/** The wait queue parents block on in wait_child(); woken when a child exits. */
WaitQueue& child_wait_queue();

/** Build @t's initial context so switch_to() starts it at @entry on @stack.
 * Returns false if the stack is too small. */
bool task_init(Task* t, void (*entry)(), uint32_t* stack, uint32_t stack_size,
               uint32_t pid);

/** Create the first user process: a fresh address space with @entry mapped at
 * the image base and a user stack, a kernel stack from the heap, and an empty
 * fd table. Returns nullptr on failure. */
Task* task_create_user(uint32_t entry);

/** Create the child of a fork: a copy-on-write clone of @parent's address
 *  space that resumes in user mode with the registers from @user_regs (a saved
 *  interrupt frame) and a zero return value. The child inherits @parent's open
 *  files. */
Task* task_create_fork_child(Task* parent, const uint32_t* user_regs);

/** fork() for the current task: returns the child's pid, or a negative errno.
 *  The child resumes just after the syscall, so this must be called from a
 *  syscall handler. */
int fork_current();

/** Add a task to the list used for parent/child bookkeeping. */
void task_register(Task* t);

/** Copy the boot user image (see init_task.cpp) to the image window. Must run
 * before the first address space is created. */
bool init_user_image();

/** Allocate a new pid for a task (used by task_create) */
uint32_t pid_alloc();

/** Current running task pid (0 if no current task) */
uint32_t current_pid();

/** Terminate the current task with the wait status @status built by
 *  sched/wait.hpp's make_wait_status_*() helpers. Never returns. */
void task_exit(int status);

/** Wait for a child of the current task to exit and collect it.
 * @pid is a pid, or -1 for any child. @status receives the exit code.
 * With @nohang, returns 0 instead of blocking when no child has exited.
 * Returns the reaped pid, 0 for WNOHANG with nothing to reap, or a negative
 * errno (-ECHILD when the task has no matching child). */
int wait_child(int32_t pid, int32_t* status, bool nohang);

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
