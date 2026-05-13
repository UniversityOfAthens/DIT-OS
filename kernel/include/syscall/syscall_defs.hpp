#ifndef KERNEL_SYSCALL_SYSCALL_DEFS_HPP
#define KERNEL_SYSCALL_SYSCALL_DEFS_HPP

#include <stdint.h>

namespace syscall
{

/** Syscall argument/return ABI: typically up to 6 args in registers, return in
 * one register */

/** Maximum number of syscall arguments (e.g. x86_64: 6) */
constexpr unsigned SYSCALL_MAX_ARGS = 6;

/** Raw arguments as passed from userspace (arch-specific layout) */
struct SyscallArgs {
    uint64_t args[SYSCALL_MAX_ARGS];
};

/** Result: value (>=0) or negated errno */
using SyscallResult = int64_t;

} // namespace syscall

#endif
