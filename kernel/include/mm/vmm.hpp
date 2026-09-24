#ifndef KERNEL_MM_VMM_HPP
#define KERNEL_MM_VMM_HPP

#include <stddef.h>
#include <stdint.h>

namespace mm
{

constexpr uint32_t VMM_IDENTITY_END = 0x01000000;
constexpr uint32_t VMM_USER_START = 0x00400000;
constexpr uint32_t VMM_USER_END = VMM_IDENTITY_END;
constexpr uint32_t USER_SPACE_COUNT = 4;
constexpr uint32_t USER_IMAGE_BASE = VMM_USER_START;
constexpr uint32_t USER_IMAGE_SIZE = 0x00100000;
constexpr uint32_t USER_STACK_BASE = 0x00B00000;
constexpr uint32_t USER_STACK_SIZE = 0x4000;

void vmm_init();
int vmm_map(uint32_t vaddr, uint32_t paddr, size_t num_pages, uint32_t flags);
void vmm_unmap(uint32_t vaddr, size_t num_pages);
uint32_t vmm_virt_to_phys(uint32_t vaddr);
uint32_t vmm_kernel_directory();
uint32_t vmm_create_user_directory();
int vmm_map_user(uint32_t directory, uint32_t vaddr, uint32_t paddr,
                 size_t num_pages, uint32_t flags);
bool vmm_user_range_ok(uint32_t directory, uint32_t vaddr, size_t size,
                       bool write);

}

#endif
