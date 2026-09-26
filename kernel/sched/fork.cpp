// DIT-OS Kernel - fork()
//
// The child gets a copy-on-write clone of the parent's address space and its
// own kernel stack, then resumes in user mode at the instruction the parent
// was interrupted on. Only the return value differs: 0 in the child, the
// child's pid in the parent.

#include "sched/sched.hpp"
#include "arch/x86_64/idt.hpp"
#include "syscall/syscall.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace sched
{

int fork_current()
{
    Task* parent = current_task;
    if (!parent || !parent->user_mode || !parent->page_directory)
        return -EINVAL;

    /* The interrupted user context, including every register the child has to
     * come back with. */
    const uint32_t* regs = syscall::current_regs();
    if (!regs)
        return -EINVAL;

    Task* child = task_create_fork_child(parent, regs);
    if (!child)
        return -ENOMEM;
    return static_cast<int>(child->pid);
}

} // namespace sched
