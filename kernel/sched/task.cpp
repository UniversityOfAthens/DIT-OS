// DIT-OS Kernel - Task creation

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "mm/kmalloc.hpp"
#include <stdint.h>
#include <stddef.h>

namespace sched
{

// Words pushed below the entry point: return trap, eip, eflags, ebx, ebp,
// esi, edi (see context.asm).
constexpr uint32_t INITIAL_FRAME_WORDS = 7;
constexpr uint32_t MIN_STACK_BYTES = 256;

// A task's entry function returns here instead of into garbage.
static void task_return_trap()
{
    task_exit(0);
}

bool task_init(Task* t, void (*entry)(), uint32_t* stack, uint32_t stack_size,
               uint32_t pid)
{
    if (!t || !stack || stack_size < MIN_STACK_BYTES)
        return false;
    stack[0] = STACK_CANARY;

    uint32_t* sp = stack + stack_size / sizeof(uint32_t) - INITIAL_FRAME_WORDS;
    sp[6] = reinterpret_cast<uint32_t>(task_return_trap);
    sp[5] = reinterpret_cast<uint32_t>(entry); // popped by ret
    sp[4] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED; // popped by popfd
    sp[3] = 0; // ebx
    sp[2] = 0; // ebp
    sp[1] = 0; // esi
    sp[0] = 0; // edi (lowest address = initial esp)

    t->ctx.edi = 0;
    t->ctx.esi = 0;
    t->ctx.ebp = 0;
    t->ctx.ebx = 0;
    t->ctx.esp = reinterpret_cast<uint32_t>(sp);
    t->ctx.eip = reinterpret_cast<uint32_t>(entry); // for debugging only
    t->state = TaskState::Runnable;
    t->next = nullptr;
    t->stack_top = reinterpret_cast<uint32_t>(stack);
    t->stack_sz = stack_size;
    t->pid = pid;
    return true;
}

Task* task_create(void (*entry)(), uint32_t* stack, uint32_t stack_size)
{
    Task* t = static_cast<Task*>(mm::kmalloc(sizeof(Task)));
    if (!t)
        return nullptr;
    if (!task_init(t, entry, stack, stack_size, pid_alloc())) {
        mm::kfree(t);
        return nullptr;
    }
    run_queue_add(t);
    return t;
}

} // namespace sched
