// DIT-OS Kernel - Syscall table and dispatch

#include "syscall/syscall_table.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_defs.hpp"
#include "errno_defs.hpp"
#include <stdint.h>

namespace syscall
{

static SyscallHandler s_table[SYS_MAX] = { nullptr };

SyscallHandler syscall_get_handler(uint32_t num)
{
    if (num >= SYS_MAX)
        return nullptr;
    return s_table[num];
}

void syscall_set_handler(uint32_t num, SyscallHandler handler)
{
    if (num >= SYS_MAX)
        return;
    s_table[num] = handler;
}

SyscallResult syscall_dispatch(uint32_t num, const SyscallArgs* args)
{
    if (num >= SYS_MAX)
        return -EINVAL;
    SyscallHandler h = s_table[num];
    if (!h)
        return -ENOSYS;
    return h(args);
}

} // namespace syscall
