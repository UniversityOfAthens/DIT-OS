#ifndef KERNEL_MM_VMM_HPP
#define KERNEL_MM_VMM_HPP

#include <stddef.h>
#include <stdint.h>

namespace mm
{

/** The kernel identity maps [0, VMM_IDENTITY_END) in its own address space and
 *  in every user address space, so any physical frame in that range can be
 *  reached by the kernel at its own address. */
constexpr uint32_t VMM_IDENTITY_END = 0x01000000;

/** Physical frames that back user pages. They sit below 4 MiB, inside that
 *  identity mapping, so the kernel can fill a frame in before any user page
 *  maps it. User pages are mapped in the window below but backed from this
 *  pool, so the pool size caps how much memory user space can hold. */
constexpr uint32_t VMM_FRAME_START = 0x00200000;
constexpr uint32_t VMM_FRAME_END = 0x00400000;

/** User virtual address window, shared by every process. */
constexpr uint32_t VMM_USER_START = 0x00400000;
constexpr uint32_t VMM_USER_END = 0x01000000;

/** The user image is mapped in full (present) in every address space, so a
 *  program's .bss and stack-adjacent code are usable without faulting. */
constexpr uint32_t USER_IMAGE_BASE = VMM_USER_START;
constexpr uint32_t USER_IMAGE_SIZE = 0x00100000;

/** Per-process user stacks: one fixed slot per address space. Processes do
 *  not need distinct stack addresses because page tables are per process. */
constexpr uint32_t USER_STACK_SIZE = 0x4000;
constexpr uint32_t USER_STACK_BASE = 0x00500000;
constexpr uint32_t USER_STACK_TOP = USER_STACK_BASE + USER_STACK_SIZE;

/** The brk() heap. Starts just above the stack and grows up. */
constexpr uint32_t USER_BRK_START = USER_STACK_TOP;
constexpr uint32_t USER_BRK_END = 0x00B00000;

/** Anonymous mmap() allocations are handed out from here upwards. */
constexpr uint32_t USER_MMAP_START = 0x00C00000;

constexpr uint32_t USER_SPACE_COUNT = 8;

void vmm_init();

/** Page directory of the kernel's own (identity mapped) address space. */
uint32_t vmm_kernel_directory();

/** Create an address space for a new process: a heap, mapping area and stack
 *  ready to be faulted in. @with_image also maps the boot program image, which
 *  only the first process needs; execve() loads its own image instead.
 * Returns the page directory address, or 0 when out of address spaces. */
uint32_t vmm_create_user_directory(bool with_image);

/** Release every frame mapped in @directory and return the space to the
 *  free list. The directory address itself is not freed (it is static). */
void vmm_destroy_user_directory(uint32_t directory);

/** Copy-on-write clone of @src_directory: pages start out shared with the
 *  source and are duplicated on the first write fault. Returns 0 on failure. */
uint32_t vmm_clone_user_directory(uint32_t src_directory);

/** True if [vaddr, vaddr+size) is mapped and accessible in @directory. */
bool vmm_user_range_ok(uint32_t directory, uint32_t vaddr, size_t size,
                       bool write);

/** Handle a user page fault: allocate a zeroed page for a region the process
 *  owns, or break copy-on-write for a shared page. Returns 0 when the fault
 *  is resolved and the instruction can be retried, negative on failure. */
int vmm_handle_fault(uint32_t directory, uint32_t vaddr, uint32_t error);

/** Copy @size bytes from the kernel into user memory, allocating or
 *  un-sharing pages as needed. Returns the number of bytes written. */
uint32_t vmm_write_user(uint32_t directory, uint32_t vaddr, const void* src,
                        uint32_t size);

/** Map one fresh anonymous page at @vaddr, bypassing the fault-time region
 *  check, and return its frame address (0 on failure). Used to load a program
 *  image into a new address space. */
uint32_t vmm_map_user_page(uint32_t directory, uint32_t vaddr);

/** brk(): set the program break, or query it with @addr == 0. Returns the
 *  resulting break, or 0 if the request is not allowed. */
uint32_t vmm_brk(uint32_t directory, uint32_t addr);

/** mmap() an anonymous region of @length bytes; returns its address or 0. */
uint32_t vmm_mmap(uint32_t directory, uint32_t length);

/** munmap() a page-aligned range; 0 on success. */
int vmm_munmap(uint32_t directory, uint32_t addr, uint32_t length);

} // namespace mm

#endif
