#ifndef KERNEL_SYSCALL_SYSCALL_HPP
#define KERNEL_SYSCALL_SYSCALL_HPP

#include <stdint.h>
#include "syscall/syscall_defs.hpp"
#include "syscall/syscall_numbers.hpp"
#include "syscall/syscall_table.hpp"
#include "syscall/syscall_trace.hpp"

namespace syscall
{

/** Initialize syscall table and arch-specific syscall entry (e.g. MSR or int
 * 0x80) */
void init();

/** Entry from arch trap/syscall: num and args from user regs; returns value to
 * put in return reg */
SyscallResult handle(uint32_t num, const SyscallArgs* args);

/** The saved frame of the syscall being handled (FRAME_* in idt.hpp), or
 * nullptr outside a syscall. Handlers that need the interrupted user context
 * (fork, execve) read it from here: it belongs to the syscall, so an
 * interrupt taken in the middle cannot displace it. */
const uint32_t* current_regs();

/** Kernel-thread wrappers (no int 0x80) */
int syscall_read(int fd, void* buf, uint32_t count);
int syscall_write(int fd, const void* buf, uint32_t count);
void syscall_exit(int code);

} // namespace syscall

#endif
