// DIT-OS Kernel - Minimal scheduler

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "mm/kmalloc.hpp"
#include "dev/serial.hpp"
#include <stdint.h>

namespace sched {

Task* current_task = nullptr;
Task* run_queue_head = nullptr;
Task* idle_task = nullptr;

static Task s_idle;
static uint32_t s_idle_stack[256];
static uint32_t s_init_stack[512];

void run_queue_add(Task* t) {
    t->next = run_queue_head;
    run_queue_head = t;
}

Task* run_queue_remove(Task* t) {
    if (run_queue_head == t) {
        run_queue_head = t->next;
        t->next = nullptr;
        return run_queue_head;
    }
    for (Task* p = run_queue_head; p && p->next; p = p->next) {
        if (p->next == t) {
            p->next = t->next;
            t->next = nullptr;
            return run_queue_head;
        }
    }
    return run_queue_head;
}

Task* schedule_next() {
    if (!run_queue_head) return idle_task;
    /* Round-robin: move head to tail, return old head */
    Task* next = run_queue_head;
    run_queue_head = run_queue_head->next;
    next->next = nullptr;
    if (run_queue_head) {
        Task* tail = run_queue_head;
        while (tail->next) tail = tail->next;
        tail->next = next;
    } else {
        run_queue_head = next;
    }
    return next;
}

void sched_init() {
    dev::serial_puts("\r\n[sched] init: heap_init\r\n");
    mm::heap_init();
    dev::serial_puts("\r\n[sched] init: idle_task_init\r\n");
    idle_task_init(&s_idle, s_idle_stack, sizeof(s_idle_stack));
    idle_task = &s_idle;
    run_queue_add(idle_task);
    dev::serial_puts("\r\n[sched] init: task_create\r\n");
    Task* init_t = task_create(get_init_entry(), s_init_stack, sizeof(s_init_stack));
    if (!init_t) {
        dev::serial_puts("\r\n[sched] init: task_create FAILED\r\n");
        for (;;) asm volatile("hlt");
    }
    dev::serial_puts("\r\n[sched] init: done\r\n");
    (void)init_t;
}

void sched_start(void (*init_entry)()) {
    (void)init_entry;
    dev::serial_puts("\r\n[sched] start: current=idle, schedule()\r\n");
    current_task = idle_task;
    schedule();
    dev::serial_puts("\r\n[sched] start: after schedule (unexpected)\r\n");
}

void schedule() {
    if (!current_task) return;

    Task* next = schedule_next();

    if (next == current_task) return;
    Task* prev = current_task;
    current_task = next;
    dev::serial_puts("\r\n[sched] switch_to\r\n");
    switch_to(prev, next);
    dev::serial_puts("\r\n[sched] switch_to returned\r\n");
}

} // namespace sched
