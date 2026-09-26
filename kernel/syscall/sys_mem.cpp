// DIT-OS Kernel - sys_brk, sys_mmap, sys_munmap
//
// All three act on the calling process' address space. None of them allocate
// physical memory: a page is only handed out when the process touches it, and
// then it is zero filled (see mm::vmm_handle_fault).

#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "sched/sched.hpp"
#include "mm/uaccess.hpp"
#include "mm/vmm.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

extern SyscallResult sys_brk_impl(const SyscallArgs* args);
extern SyscallResult sys_mmap_impl(const SyscallArgs* args);
extern SyscallResult sys_munmap_impl(const SyscallArgs* args);

static uint32_t current_directory()
{
    return sched::current_task ? sched::current_task->page_directory : 0;
}

/* Values from the mmap ABI (userland/include/syscall.h). Only read/write
 * anonymous private memory is implemented, so anything else is rejected
 * rather than silently ignored. */
constexpr uint32_t PROT_READ = 0x1;
constexpr uint32_t PROT_WRITE = 0x2;
constexpr uint32_t MAP_PRIVATE = 0x02;
constexpr uint32_t MAP_ANONYMOUS = 0x20;
constexpr uint32_t MMAP_SUPPORTED = MAP_PRIVATE | MAP_ANONYMOUS;

SyscallResult sys_brk_impl(const SyscallArgs* args)
{
    uint32_t directory = current_directory();
    if (!directory)
        return -ESRCH;
    /* A failed request returns the unchanged break, so the result is only
     * zero when the call itself is invalid. */
    return static_cast<SyscallResult>(
        mm::vmm_brk(directory, static_cast<uint32_t>(args->args[0])));
}

SyscallResult sys_mmap_impl(const SyscallArgs* args)
{
    uint32_t directory = current_directory();
    if (!directory)
        return -ESRCH;
    uintptr_t addr = static_cast<uintptr_t>(args->args[0]);
    uint32_t length = static_cast<uint32_t>(args->args[1]);
    uint32_t prot = static_cast<uint32_t>(args->args[2]);
    uint32_t flags = static_cast<uint32_t>(args->args[3]);

    if (addr != 0 || prot != (PROT_READ | PROT_WRITE) ||
        flags != MMAP_SUPPORTED)
        return -EINVAL;
    uint32_t mapped = mm::vmm_mmap(directory, length);
    if (!mapped)
        return -ENOMEM;
    return static_cast<SyscallResult>(mapped);
}

SyscallResult sys_munmap_impl(const SyscallArgs* args)
{
    uint32_t directory = current_directory();
    if (!directory)
        return -ESRCH;
    uint32_t addr = static_cast<uint32_t>(args->args[0]);
    uint32_t length = static_cast<uint32_t>(args->args[1]);
    return mm::vmm_munmap(directory, addr, length);
}

} // namespace syscall
