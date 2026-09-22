#ifndef KERNEL_MM_UACCESS_HPP
#define KERNEL_MM_UACCESS_HPP

// DIT-OS Kernel - Checked access to user memory from syscalls
//
// Every pointer a syscall receives must go through these helpers instead of
// being dereferenced directly. They reject NULL/low memory, ranges that wrap
// around the address space, and anything outside the user window.
//
// Limitation (ISSUES.md C5/I3): userland still runs in ring 0 inside the
// kernel image, so its buffers live in kernel memory and the window has to
// include the kernel. Once user programs run in ring 3 with their own
// address space, only USER_SPACE_START/END need to change.

#include <stddef.h>
#include <stdint.h>

namespace mm
{

/** Below 1 MiB: IVT, BIOS data, bootloader, VGA memory; never user memory. */
constexpr uintptr_t USER_SPACE_START = 0x00100000;
/** Conventional 3 GiB split; the upper 1 GiB is reserved for the kernel. */
constexpr uintptr_t USER_SPACE_END = 0xC0000000;

/** True if [@addr, @addr + @size) lies entirely inside the user window.
 * A zero-length range is valid anywhere. */
bool user_range_ok(uintptr_t addr, size_t size);

/** Copy @size bytes from user @src to kernel @dst. 0 or -EFAULT. */
int copy_from_user(void* dst, uintptr_t src, size_t size);

/** Copy @size bytes from kernel @src to user @dst. 0 or -EFAULT. */
int copy_to_user(uintptr_t dst, const void* src, size_t size);

/** Copy a NUL-terminated user string into @dst (capacity @cap, >= 1).
 * Returns the length, -EFAULT for a bad pointer, or -ENAMETOOLONG if no NUL
 * is found within @cap - 1 bytes. */
int strncpy_from_user(char* dst, uintptr_t src, size_t cap);

} // namespace mm

#endif // KERNEL_MM_UACCESS_HPP
