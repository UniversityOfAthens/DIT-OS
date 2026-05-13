// DIT-OS Kernel - Syscall trace (stub: no-op unless enabled)

#include "syscall/syscall_trace.hpp"
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include <stdint.h>

namespace syscall
{

static int s_trace_enabled = 0;

void trace_enter(uint32_t num, const SyscallArgs* args)
{
    (void)num;
    (void)args;
    if (!s_trace_enabled)
        return;
}

void trace_exit(uint32_t num, SyscallResult result)
{
    (void)num;
    (void)result;
    if (!s_trace_enabled)
        return;
}

void trace_enable(int on)
{
    s_trace_enabled = on ? 1 : 0;
}

} // namespace syscall
