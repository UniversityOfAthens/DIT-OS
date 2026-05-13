// DIT-OS Kernel - Task creation

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "mm/kmalloc.hpp"
#include <stdint.h>
#include <stddef.h>

namespace sched
{

Task* task_create(void (*entry)(), uint32_t* stack, uint32_t stack_size)
{
    if (stack_size < 32)
        return nullptr;
    uint32_t* sp = stack + stack_size / sizeof(uint32_t);
    sp--;
    *sp = 0; // EBX (popped 4th by context.asm)
    sp--;
    *sp = 0; // EBP (popped 3rd)
    sp--;
    *sp = 0; // ESI (popped 2nd)
    sp--;
    *sp = 0; // EDI (popped 1st)
    sp--;
    *sp = (uint32_t)entry; // EIP (popped by ret)
    Task* t = (Task*)mm::heap_alloc(sizeof(Task));
    if (!t)
        return nullptr;
    t->ctx.edi = 0;
    t->ctx.esi = 0;
    t->ctx.ebp = 0;
    t->ctx.ebx = 0;
    t->ctx.esp = (uint32_t)sp;
    t->ctx.eip =
        (uint32_t)entry; /* for debugging; context.asm resumes via stack+ret */
    t->state = TaskState::Runnable;
    t->next = nullptr;
    t->stack_top = (uint32_t)stack;
    t->stack_sz = stack_size;
    t->pid = pid_alloc();
    run_queue_add(t);
    return t;
}

} // namespace sched
