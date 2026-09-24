// DIT-OS Kernel - Task creation

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "mm/kmalloc.hpp"
#include "mm/vmm.hpp"
#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/paging.hpp"
#include <stdint.h>
#include <stddef.h>

extern "C" void user_entry_trap();

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
    t->page_directory = 0;
    t->user_stack_top = 0;
    t->user_mode = false;
    return true;
}

Task* task_create_user(uint32_t entry, uint32_t* kernel_stack,
                       uint32_t kernel_stack_size, uint32_t* user_stack,
                       uint32_t user_stack_size)
{
    if (!kernel_stack || !user_stack || kernel_stack_size < MIN_STACK_BYTES ||
        user_stack_size < 36)
        return nullptr;

    kernel_stack[0] = STACK_CANARY;

    uint32_t* user_frame = user_stack + user_stack_size / sizeof(uint32_t) - 9;
    uint32_t user_frame_vaddr =
        mm::USER_STACK_BASE + user_stack_size - 9 * sizeof(uint32_t);
    user_frame[0] = entry;
    user_frame[1] = arch::GDT_SEL_USER_CODE;
    user_frame[2] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED;
    user_frame[3] = user_frame_vaddr + 5 * sizeof(uint32_t);
    user_frame[4] = arch::GDT_SEL_USER_DATA;
    user_frame[5] = 0;
    user_frame[6] = 0;
    user_frame[7] = 0;
    user_frame[8] = 0;

    Task* t = static_cast<Task*>(mm::kmalloc(sizeof(Task)));
    if (!t)
        return nullptr;

    uint32_t directory = mm::vmm_create_user_directory();
    if (!directory)
    {
        mm::kfree(t);
        return nullptr;
    }
    if (mm::vmm_map_user(directory, mm::USER_IMAGE_BASE, mm::USER_IMAGE_BASE,
                         mm::USER_IMAGE_SIZE / arch::PAGE_SIZE,
                         arch::PTE_US | arch::PTE_RW) != 0 ||
        mm::vmm_map_user(directory, mm::USER_STACK_BASE,
                         reinterpret_cast<uint32_t>(user_stack),
                         user_stack_size / arch::PAGE_SIZE,
                         arch::PTE_US | arch::PTE_RW) != 0)
    {
        mm::kfree(t);
        return nullptr;
    }

    uint32_t* sp = kernel_stack + kernel_stack_size / sizeof(uint32_t) -
                   INITIAL_FRAME_WORDS;
    sp[6] = reinterpret_cast<uint32_t>(task_return_trap);
    sp[5] = reinterpret_cast<uint32_t>(user_entry_trap);
    sp[4] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED;
    sp[3] = 0;
    sp[2] = 0;
    sp[1] = 0;
    sp[0] = user_frame_vaddr;

    t->ctx.edi = user_frame_vaddr;
    t->ctx.esi = 0;
    t->ctx.ebp = 0;
    t->ctx.ebx = 0;
    t->ctx.esp = reinterpret_cast<uint32_t>(sp);
    t->ctx.eip = reinterpret_cast<uint32_t>(user_entry_trap);
    t->state = TaskState::Runnable;
    t->next = nullptr;
    t->stack_top = reinterpret_cast<uint32_t>(kernel_stack);
    t->stack_sz = kernel_stack_size;
    t->pid = pid_alloc();
    t->page_directory = directory;
    t->user_stack_top = user_frame_vaddr + 5 * sizeof(uint32_t);
    t->user_mode = true;

    run_queue_add(t);
    return t;
}

} // namespace sched
