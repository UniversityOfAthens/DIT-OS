// DIT-OS Kernel - Idle task

#include "sched/sched.hpp"
#include "dev/serial.hpp"
#include <stdint.h>

namespace sched
{

static void idle_loop()
{
    for (;;) {
        asm volatile("sti; hlt");
    }
}

void idle_task_init(Task* idle, uint32_t* stack, uint32_t stack_size)
{
    stack[0] = STACK_CANARY;
    uint32_t* sp = stack + stack_size / sizeof(uint32_t);
    sp--;
    *sp = (uint32_t)idle_loop;
    sp--;
    *sp = 0;
    sp--;
    *sp = 0;
    sp--;
    *sp = 0;
    sp--;
    *sp = 0;
    idle->ctx.edi = 0;
    idle->ctx.esi = 0;
    idle->ctx.ebp = 0;
    idle->ctx.ebx = 0;
    idle->ctx.esp = (uint32_t)sp;
    idle->ctx.eip = (uint32_t)
        idle_loop; /* for debugging; context.asm resumes via stack+ret */
    idle->state = TaskState::Runnable;
    idle->next = nullptr;
    idle->stack_top = (uint32_t)stack;
    idle->stack_sz = stack_size;
    idle->pid = 0;
}

} // namespace sched
