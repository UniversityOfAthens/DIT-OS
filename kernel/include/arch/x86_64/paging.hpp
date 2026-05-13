#ifndef KERNEL_ARCH_X86_64_PAGING_HPP
#define KERNEL_ARCH_X86_64_PAGING_HPP

#include <stdint.h>

namespace arch
{

// 32-bit paging: 4KB pages, 2-level (PD + PT), 1024 entries each
constexpr unsigned PAGE_SIZE = 4096;
constexpr unsigned PAGE_SHIFT = 12;
constexpr unsigned PD_ENTRIES = 1024;
constexpr unsigned PT_ENTRIES = 1024;

// Page directory / table entry flags
constexpr uint32_t PTE_P = 1u << 0; // Present
constexpr uint32_t PTE_RW = 1u << 1; // Read/Write
constexpr uint32_t PTE_US = 1u << 2; // User/Supervisor
constexpr uint32_t PTE_PWT = 1u << 3; // Page Write-Through
constexpr uint32_t PTE_PCD = 1u << 4; // Page Cache Disable
constexpr uint32_t PTE_A = 1u << 5; // Accessed
constexpr uint32_t PTE_D = 1u << 6; // Dirty (PTE only)
constexpr uint32_t PTE_PS = 1u << 7; // Page Size (PDE: 4MB page)
constexpr uint32_t PTE_G = 1u << 8; // Global

// Default kernel mapping flags
constexpr uint32_t PTE_KERNEL = PTE_P | PTE_RW;
constexpr uint32_t PDE_KERNEL = PTE_P | PTE_RW;

// Load page directory (physical address) into CR3
void paging_load(uint32_t pd_physical);

// Invalidate a single TLB entry for virtual address
void tlb_invalidate(uint32_t vaddr);

// Flush entire TLB
void tlb_flush();

// Enable paging (set CR0.PG). Call after loading CR3 and mapping kernel.
void paging_enable();

// Disable paging
void paging_disable();

} // namespace arch

#endif
