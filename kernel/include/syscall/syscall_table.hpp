#ifndef KERNEL_SYSCALL_SYSCALL_TABLE_HPP
#define KERNEL_SYSCALL_SYSCALL_TABLE_HPP

#include <stdint.h>
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"

namespace syscall {

/** Handler: (args) -> result. Implemented in kernel, called from arch trap/syscall entry */
using SyscallHandler = SyscallResult (*)(const SyscallArgs* args);

/** Get handler for syscall number; nullptr if invalid */
SyscallHandler syscall_get_handler(uint32_t num);

/** Register or override handler for a syscall number (e.g. for testing) */
void syscall_set_handler(uint32_t num, SyscallHandler handler);

/** Dispatch: call handler for num, return result */
SyscallResult syscall_dispatch(uint32_t num, const SyscallArgs* args);

} // namespace syscall

#endif
