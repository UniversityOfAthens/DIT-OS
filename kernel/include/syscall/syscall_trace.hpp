#ifndef KERNEL_SYSCALL_SYSCALL_TRACE_HPP
#define KERNEL_SYSCALL_SYSCALL_TRACE_HPP

#include <stdint.h>
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"

namespace syscall {

/** Optional: log syscall entry/exit (for debugging). No-op if tracing disabled */
void trace_enter(uint32_t num, const SyscallArgs* args);

/** Optional: log syscall return value */
void trace_exit(uint32_t num, SyscallResult result);

/** Enable/disable tracing (e.g. per-process or global flag) */
void trace_enable(int on);

} // namespace syscall

#endif
