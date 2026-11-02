// DIT-OS Kernel - Task creation

#include "sched/sched.hpp"
#include "sched/task.hpp"
#include "sched/wait.hpp"
#include "arch/x86_64/irqflags.hpp"
#include "mm/kmalloc.hpp"
#include "mm/vmm.hpp"
#include "syscall/fd.hpp"
#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/paging.hpp"
#include <stdint.h>
#include <stddef.h>

extern "C" void user_entry_trap();
extern "C" void fork_entry_trap();

namespace sched
{

// Words pushed below the entry point: return trap, eip, eflags, ebx, ebp,
// esi, edi (see context.asm).
constexpr uint32_t INITIAL_FRAME_WORDS = 7;
constexpr uint32_t MIN_STACK_BYTES = 256;

/* Kernel stack for a task. The syscall path is shallow (256 byte bounce
 * buffers), so 8 KiB is comfortable. */
constexpr uint32_t KERNEL_STACK_WORDS = 2048;
constexpr uint32_t KERNEL_STACK_BYTES = KERNEL_STACK_WORDS * sizeof(uint32_t);

// A task's entry function returns here instead of into garbage.
static void task_return_trap()
{
    task_exit(make_wait_status_exit(0));
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
    t->all_next = nullptr;
    t->stack_top = reinterpret_cast<uint32_t>(stack);
    t->stack_sz = stack_size;
    t->pid = pid;
    t->page_directory = 0;
    t->user_stack_top = 0;
    t->user_mode = false;
    t->ppid = 0;
    t->exit_code = 0;
    t->fd_table = -1;
    t->pending_signals = 0;
    t->wait_queue = nullptr;
    return true;
}

namespace
{

/* Build the frame that returns to user mode at @entry with stack pointer
 * @esp, written just below @esp so the first user instruction sees an
 * otherwise empty stack. Returns the address of the frame, which is what the
 * entry stub loads into ESP (iret pops the frame and puts @esp back), or 0 if
 * the pages cannot be faulted in. */
uint32_t build_user_frame(uint32_t directory, uint32_t entry, uint32_t esp)
{
    uint32_t frame[5];
    /* iret pops from increasing addresses, so eip comes first. The privilege
     * level changes, so esp and ss are part of the frame too. */
    frame[0] = entry; // eip
    frame[1] = arch::GDT_SEL_USER_CODE; // cs
    frame[2] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED;
    frame[3] = esp; // esp
    frame[4] = arch::GDT_SEL_USER_DATA; // ss

    uint32_t at = esp - sizeof(frame);
    if (at < mm::USER_STACK_TOP - mm::USER_STACK_SIZE)
        return 0;
    if (mm::vmm_write_user(directory, at, frame, sizeof(frame)) !=
        sizeof(frame))
        return 0;
    return at;
}

/* Point @t at a fresh kernel stack whose first instruction is @eip, with EDI
 * holding the user iret frame to enter through and ESI an optional image of
 * the user registers to restore (see fork_entry_trap). */
bool setup_user_context(Task* t, uint32_t* stack, uint32_t stack_size,
                        uint32_t eip, uint32_t frame, uint32_t user_esi)
{
    if (!stack || stack_size < MIN_STACK_BYTES)
        return false;
    stack[0] = STACK_CANARY;

    uint32_t* sp = stack + stack_size / sizeof(uint32_t) - INITIAL_FRAME_WORDS;
    sp[6] = reinterpret_cast<uint32_t>(task_return_trap);
    sp[5] = eip;
    sp[4] = arch::EFLAGS_IF | arch::EFLAGS_RESERVED;
    sp[3] = 0;
    sp[2] = 0;
    sp[1] = user_esi;
    sp[0] = frame; // lowest address = initial esp, read as EDI

    t->ctx.edi = frame;
    t->ctx.esi = user_esi;
    t->ctx.ebp = 0;
    t->ctx.ebx = 0;
    t->ctx.esp = reinterpret_cast<uint32_t>(sp);
    t->ctx.eip = eip;
    return true;
}

} // namespace

Task* task_create_user(uint32_t entry)
{
    uint32_t directory = mm::vmm_create_user_directory(true);
    if (!directory)
        return nullptr;
    /* The stack starts with room for the iret frame, and the initial stack
     * pointer stays 16 byte aligned. */
    uint32_t esp = mm::USER_STACK_TOP - 16;
    uint32_t frame = build_user_frame(directory, entry, esp);
    if (!frame)
    {
        mm::vmm_destroy_user_directory(directory);
        return nullptr;
    }

    Task* t = static_cast<Task*>(mm::kmalloc(sizeof(Task)));
    uint32_t* stack = static_cast<uint32_t*>(mm::kmalloc(KERNEL_STACK_BYTES));
    if (!t || !stack)
    {
        mm::kfree(t);
        mm::kfree(stack);
        mm::vmm_destroy_user_directory(directory);
        return nullptr;
    }

    for (size_t i = 0; i < sizeof(Task) / sizeof(uint32_t); i++)
        reinterpret_cast<uint32_t*>(t)[i] = 0;
    t->fd_table = syscall::fd_table_create();

    if (!setup_user_context(t, stack, KERNEL_STACK_BYTES,
                            reinterpret_cast<uint32_t>(user_entry_trap), frame,
                            0))
    {
        syscall::fd_table_release(t->fd_table);
        mm::kfree(t);
        mm::kfree(stack);
        mm::vmm_destroy_user_directory(directory);
        return nullptr;
    }

    t->state = TaskState::Runnable;
    t->next = nullptr;
    t->all_next = nullptr;
    t->stack_top = reinterpret_cast<uint32_t>(stack);
    t->stack_sz = KERNEL_STACK_BYTES;
    t->pid = pid_alloc();
    t->page_directory = directory;
    t->user_stack_top = esp;
    t->user_mode = true;
    t->ppid = 0;
    t->exit_code = 0;

    task_register(t);
    run_queue_add(t);
    return t;
}

Task* task_create_fork_child(Task* parent, const uint32_t* user_regs)
{
    if (!parent || !parent->page_directory || !user_regs)
        return nullptr;
    /* Cloning first: it must see the parent's mappings, and the parent keeps
     * running in its own (now copy-on-write) space. */
    uint32_t directory = mm::vmm_clone_user_directory(parent->page_directory);
    if (!directory)
        return nullptr;

    Task* t = static_cast<Task*>(mm::kmalloc(sizeof(Task)));
    uint32_t* stack = static_cast<uint32_t*>(mm::kmalloc(KERNEL_STACK_BYTES));
    if (!t || !stack)
    {
        mm::kfree(t);
        mm::kfree(stack);
        mm::vmm_destroy_user_directory(directory);
        return nullptr;
    }

    /* The parent's user registers, kept at the bottom of the child's kernel
     * stack (above the canary) for fork_entry_trap to restore. */
    uint32_t* image = stack + 1;
    image[0] = user_regs[arch::FRAME_EDI];
    image[1] = user_regs[arch::FRAME_ESI];
    image[2] = user_regs[arch::FRAME_EBP];
    image[3] = user_regs[arch::FRAME_STACK_ESP];
    image[4] = user_regs[arch::FRAME_EBX];
    image[5] = user_regs[arch::FRAME_EDX];
    image[6] = user_regs[arch::FRAME_ECX];
    uint32_t esp = user_regs[arch::FRAME_STACK_ESP];

    for (size_t i = 0; i < sizeof(Task) / sizeof(uint32_t); i++)
        reinterpret_cast<uint32_t*>(t)[i] = 0;

    /* The child returns to user mode at the parent's interrupted EIP, on the
     * parent's stack, which the clone gave it its own copy of. Writing the
     * frame here is what makes that copy private. */
    uint32_t frame =
        build_user_frame(directory, user_regs[arch::FRAME_EIP], esp);
    if (!frame)
    {
        mm::kfree(t);
        mm::kfree(stack);
        mm::vmm_destroy_user_directory(directory);
        return nullptr;
    }

    if (!setup_user_context(t, stack, KERNEL_STACK_BYTES,
                            reinterpret_cast<uint32_t>(fork_entry_trap), frame,
                            reinterpret_cast<uint32_t>(image)))
    {
        mm::kfree(t);
        mm::kfree(stack);
        mm::vmm_destroy_user_directory(directory);
        return nullptr;
    }

    t->state = TaskState::Runnable;
    t->next = nullptr;
    t->all_next = nullptr;
    t->stack_top = reinterpret_cast<uint32_t>(stack);
    t->stack_sz = KERNEL_STACK_BYTES;
    t->pid = pid_alloc();
    t->page_directory = directory;
    t->user_stack_top = esp;
    t->user_mode = true;
    t->ppid = parent->pid;
    t->exit_code = 0;
    t->fd_table = syscall::fd_table_dup(parent->fd_table);

    task_register(t);
    run_queue_add(t);
    return t;
}

} // namespace sched
