// DIT-OS Kernel - Minimal scheduler

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "sched/signal.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "arch/x86_64/paging.hpp"
#include "arch/x86_64/tss.hpp"
#include "core/panic.hpp"
#include "mm/kmalloc.hpp"
#include "mm/vmm.hpp"
#include "dev/serial.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace sched
{

Task* current_task = nullptr;
Task* run_queue_head = nullptr;
Task* idle_task = nullptr;

/* Every task that has not been reaped, so a parent can find its children and
 * wait for them. */
static Task* s_all_tasks = nullptr;
/* Parents blocked in wait_child(). */
static WaitQueue s_child_wait;

WaitQueue& child_wait_queue()
{
    return s_child_wait;
}

static Task s_idle;
/* Idle also takes IRQs (serial, timer) on this stack. */
static uint32_t s_idle_stack[1024];

void task_register(Task* t)
{
    if (!t)
        return;
    t->all_next = s_all_tasks;
    s_all_tasks = t;
}

static void task_unregister(Task* t)
{
    Task** link = &s_all_tasks;
    while (*link)
    {
        if (*link == t)
        {
            *link = t->all_next;
            t->all_next = nullptr;
            return;
        }
        link = &(*link)->all_next;
    }
}

Task* task_find(int32_t pid)
{
    if (pid <= 0)
        return nullptr;
    uint32_t wanted = static_cast<uint32_t>(pid);
    for (Task* t = s_all_tasks; t; t = t->all_next)
    {
        if (t->pid == wanted)
            return t;
    }
    return nullptr;
}

void task_reparent_children(uint32_t old_pid, uint32_t new_pid)
{
    for (Task* t = s_all_tasks; t; t = t->all_next)
    {
        if (t->ppid == old_pid)
            t->ppid = new_pid;
    }
}

/* Unlink @t from the wait queue recorded on the task. */
static void wait_queue_remove(Task* t)
{
    WaitQueue* wq = t->wait_queue;
    if (!wq)
        return;
    Task** link = &wq->head;
    while (*link && *link != t)
        link = &(*link)->next;
    if (*link == t)
        *link = t->next;
    t->wait_queue = nullptr;
}

void task_wake(Task* t)
{
    if (!t || t == idle_task || t->state != TaskState::Blocked)
        return;
    wait_queue_remove(t);
    t->next = nullptr;
    t->state = TaskState::Runnable;
    run_queue_add(t);
}

/* The exited child of @parent matching @pid (-1 for any). */
static Task* find_exited_child(Task* parent, int32_t pid)
{
    for (Task* t = s_all_tasks; t; t = t->all_next)
    {
        if (t->ppid != parent->pid || t->state != TaskState::Exited)
            continue;
        if (pid < 0 || static_cast<int32_t>(t->pid) == pid)
            return t;
    }
    return nullptr;
}

static bool has_child(Task* parent, int32_t pid)
{
    for (Task* t = s_all_tasks; t; t = t->all_next)
    {
        if (t->ppid != parent->pid)
            continue;
        if (pid < 0 || static_cast<int32_t>(t->pid) == pid)
            return true;
    }
    return false;
}

int wait_child(int32_t pid, int32_t* status, bool nohang)
{
    Task* parent = current_task;
    if (!parent)
        return -ECHILD;
    if (pid == 0)
        return -ECHILD;

    for (;;)
    {
        /* Checking for an exit and going to sleep happen with IRQs off, so a
         * child that exits in between still wakes us. */
        uint32_t flags = arch::irq_save();
        Task* child = find_exited_child(parent, pid);
        if (child)
        {
            int32_t code = child->exit_code;
            uint32_t child_pid = child->pid;
            /* The child is unreachable: release its stack and bookkeeping. */
            task_unregister(child);
            mm::kfree(reinterpret_cast<void*>(child->stack_top));
            mm::kfree(child);
            arch::irq_restore(flags);
            if (status)
                *status = code;
            return static_cast<int>(child_pid);
        }
        if (!has_child(parent, pid))
        {
            arch::irq_restore(flags);
            return -ECHILD;
        }
        if (nohang)
        {
            arch::irq_restore(flags);
            return 0;
        }
        sleep_on(&s_child_wait);
        arch::irq_restore(flags);
    }
}

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
    Task* init_t = task_create_user(mm::USER_IMAGE_BASE);
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
    /* A fatal signal must not be lost to an indefinite sleep. */
    signal_check_current();
    t->state = TaskState::Blocked;
    run_queue_remove(t);
    t->wait_queue = wq;
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
        t->wait_queue = nullptr;
        t->state = TaskState::Runnable;
        run_queue_add(t);
        t = next;
    }
    arch::irq_restore(flags);
}

} // namespace sched
