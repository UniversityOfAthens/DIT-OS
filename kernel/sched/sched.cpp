// DIT-OS Kernel - Minimal scheduler

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "arch/x86_64/paging.hpp"
#include "arch/x86_64/tss.hpp"
#include "core/panic.hpp"
#include "mm/kmalloc.hpp"
#include "mm/vmm.hpp"
#include "dev/serial.hpp"
#include <stdint.h>

namespace sched
{

Task* current_task = nullptr;
Task* run_queue_head = nullptr;
Task* idle_task = nullptr;

static Task s_idle;
/* Idle also takes IRQs (serial, timer) on this stack. */
static uint32_t s_idle_stack[1024];
/* The init task runs the shell and the whole syscall/VFS path on this stack. */
static uint32_t s_init_stack[4096];
static uint32_t s_user_stack[mm::USER_STACK_SIZE / 4]
    __attribute__((aligned(4096)));

void run_queue_add(Task* t)
{
    t->next = run_queue_head;
    run_queue_head = t;
}

Task* run_queue_remove(Task* t)
{
    if (run_queue_head == t)
    {
        run_queue_head = t->next;
        t->next = nullptr;
        return run_queue_head;
    }
    for (Task* p = run_queue_head; p && p->next; p = p->next)
    {
        if (p->next == t)
        {
            p->next = t->next;
            t->next = nullptr;
            return run_queue_head;
        }
    }
    return run_queue_head;
}

Task* schedule_next()
{
    if (!run_queue_head)
        return idle_task;
    /* Round-robin: move head to tail, return old head */
    Task* next = run_queue_head;
    run_queue_head = run_queue_head->next;
    next->next = nullptr;
    if (run_queue_head)
    {
        Task* tail = run_queue_head;
        while (tail->next)
            tail = tail->next;
        tail->next = next;
    }
    else
    {
        run_queue_head = next;
    }
    return next;
}

void task_activate(Task* task)
{
    if (!task)
        core::panic("task_activate: null task");
    uint32_t directory = task->page_directory;
    if (!directory)
        directory = mm::vmm_kernel_directory();
    arch::paging_load(directory);
    arch::tss_set_kernel_stack(task->stack_top + task->stack_sz);
    current_task = task;
}

void sched_init()
{
    dev::serial_puts("\r\n[sched] init: heap_init\r\n");
    mm::heap_init();
    dev::serial_puts("\r\n[sched] init: idle_task_init\r\n");
    idle_task_init(&s_idle, s_idle_stack, sizeof(s_idle_stack));
    idle_task = &s_idle;
    run_queue_add(idle_task);
    dev::serial_puts("\r\n[sched] init: task_create\r\n");
    if (!init_user_image())
    {
        dev::serial_puts("\r\n[sched] init: user image FAILED\r\n");
        for (;;)
            asm volatile("hlt");
    }
    Task* init_t = task_create_user(mm::USER_IMAGE_BASE, s_init_stack,
                                    sizeof(s_init_stack), s_user_stack,
                                    sizeof(s_user_stack));
    if (!init_t)
    {
        dev::serial_puts("\r\n[sched] init: task_create FAILED\r\n");
        for (;;)
            asm volatile("hlt");
    }
    dev::serial_puts("\r\n[sched] init: done\r\n");
    (void)init_t;
}

void sched_start(void (*init_entry)())
{
    (void)init_entry;
    dev::serial_puts("\r\n[sched] start: leaving boot stack\r\n");
    // Switch away from the boot context for good: it is saved into a task
    // that is never on the run queue, so it is never resumed.
    static Task s_boot_task;
    arch::irq_save();
    current_task = &s_boot_task;
    Task* first = schedule_next();
    task_activate(first);
    switch_to(&s_boot_task, first);
    core::panic("sched_start: boot context resumed");
}

static void check_stack(const Task* t)
{
    if (t->stack_top &&
        *reinterpret_cast<const uint32_t*>(t->stack_top) != STACK_CANARY)
        core::panicf("stack overflow in task pid %u\n", t->pid);
}

bool has_other_runnable()
{
    return run_queue_head &&
           (run_queue_head != idle_task || run_queue_head->next);
}

// Switch to the next task on the run queue. Interrupts must be disabled.
static void switch_to_next()
{
    Task* prev = current_task;
    Task* next = schedule_next();
    if (next == prev)
        return;
    check_stack(prev);
    task_activate(next);
    switch_to(prev, next);
}

void schedule()
{
    if (!current_task)
        return;
    // The run queue is also changed by IRQs (wake_up_all), so keep them off
    // while picking and switching; switch_to saves and restores EFLAGS.
    uint32_t flags = arch::irq_save();
    switch_to_next();
    arch::irq_restore(flags);
}

void sleep_on(WaitQueue* wq)
{
    Task* t = current_task;
    if (!t || t == idle_task)
        core::panic("sleep_on: no task context to block");
    t->state = TaskState::Blocked;
    run_queue_remove(t);
    t->next = wq->head;
    wq->head = t;
    switch_to_next();
}

void wake_up_all(WaitQueue* wq)
{
    uint32_t flags = arch::irq_save();
    Task* t = wq->head;
    wq->head = nullptr;
    while (t)
    {
        Task* next = t->next;
        t->state = TaskState::Runnable;
        run_queue_add(t);
        t = next;
    }
    arch::irq_restore(flags);
}

} // namespace sched
