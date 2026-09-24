#include "mm/vmm.hpp"
#include "arch/x86_64/paging.hpp"
#include <stddef.h>
#include <stdint.h>

namespace mm
{

namespace
{

using arch::PAGE_SHIFT;
using arch::PAGE_SIZE;
using arch::PDE_KERNEL;
using arch::PD_ENTRIES;
using arch::PTE_P;
using arch::PTE_KERNEL;
using arch::PTE_RW;
using arch::PTE_US;
using arch::PT_ENTRIES;

constexpr uint32_t IDENTITY_PAGE_COUNT = VMM_IDENTITY_END / PAGE_SIZE;
constexpr uint32_t IDENTITY_PDE_COUNT = IDENTITY_PAGE_COUNT / PT_ENTRIES;

struct alignas(PAGE_SIZE) AddressSpace
{
    uint32_t directory[PD_ENTRIES];
    uint32_t tables[IDENTITY_PDE_COUNT][PT_ENTRIES];
};

static AddressSpace s_kernel_space;
static AddressSpace s_user_spaces[USER_SPACE_COUNT];
static bool s_user_directory_used[USER_SPACE_COUNT];
static bool s_initialized;

void clear_space(AddressSpace& space)
{
    for (uint32_t i = 0; i < PD_ENTRIES; i++)
        space.directory[i] = 0;
    for (uint32_t i = 0; i < IDENTITY_PDE_COUNT; i++)
    {
        for (uint32_t j = 0; j < PT_ENTRIES; j++)
            space.tables[i][j] = 0;
    }
}

void map_identity(AddressSpace& space)
{
    for (uint32_t table_index = 0; table_index < IDENTITY_PDE_COUNT;
         table_index++)
    {
        uint32_t* table = space.tables[table_index];
        for (uint32_t entry = 0; entry < PT_ENTRIES; entry++)
        {
            uint32_t virtual_page = table_index * PT_ENTRIES + entry;
            table[entry] = (virtual_page << PAGE_SHIFT) | PTE_KERNEL;
        }
        space.directory[table_index] = reinterpret_cast<uint32_t>(table) |
                                       PDE_KERNEL;
    }
}

int map_range(AddressSpace& space, uint32_t vaddr, uint32_t paddr,
              size_t num_pages, uint32_t flags)
{
    if (!s_initialized || num_pages == 0)
        return -1;
    if ((vaddr & (PAGE_SIZE - 1)) != 0 || (paddr & (PAGE_SIZE - 1)) != 0)
        return -1;
    if (vaddr >= VMM_IDENTITY_END)
        return -1;
    size_t bytes = num_pages * PAGE_SIZE;
    if (bytes > VMM_IDENTITY_END - vaddr)
        return -1;

    for (size_t i = 0; i < num_pages; i++)
    {
        uint32_t current_vaddr = vaddr + static_cast<uint32_t>(i * PAGE_SIZE);
        uint32_t current_paddr = paddr + static_cast<uint32_t>(i * PAGE_SIZE);
        uint32_t directory_index = current_vaddr >> 22;
        uint32_t table_index = (current_vaddr >> PAGE_SHIFT) & (PT_ENTRIES - 1);
        uint32_t* table = space.tables[directory_index];
        if (!(space.directory[directory_index] & PTE_P))
            return -1;
        if (flags & PTE_US)
            space.directory[directory_index] |= PTE_US;
        table[table_index] = current_paddr | PTE_P | flags;
    }
    return 0;
}

void unmap_range(AddressSpace& space, uint32_t vaddr, size_t num_pages)
{
    if (!s_initialized || num_pages == 0 || (vaddr & (PAGE_SIZE - 1)) != 0)
        return;
    if (vaddr >= VMM_IDENTITY_END)
        return;
    size_t bytes = num_pages * PAGE_SIZE;
    if (bytes > VMM_IDENTITY_END - vaddr)
        return;

    for (size_t i = 0; i < num_pages; i++)
    {
        uint32_t current_vaddr = vaddr + static_cast<uint32_t>(i * PAGE_SIZE);
        uint32_t directory_index = current_vaddr >> 22;
        uint32_t table_index = (current_vaddr >> PAGE_SHIFT) & (PT_ENTRIES - 1);
        if (space.directory[directory_index] & PTE_P)
            space.tables[directory_index][table_index] = 0;
    }
}

uint32_t directory_address(const AddressSpace& space)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(space.directory));
}

AddressSpace* user_space_for_directory(uint32_t directory)
{
    for (uint32_t i = 0; i < USER_SPACE_COUNT; i++)
    {
        if (s_user_directory_used[i] &&
            directory_address(s_user_spaces[i]) == directory)
            return &s_user_spaces[i];
    }
    return nullptr;
}

}

void vmm_init()
{
    clear_space(s_kernel_space);
    for (uint32_t i = 0; i < USER_SPACE_COUNT; i++)
    {
        clear_space(s_user_spaces[i]);
        map_identity(s_user_spaces[i]);
        s_user_directory_used[i] = false;
    }
    map_identity(s_kernel_space);
    s_initialized = true;
}

int vmm_map(uint32_t vaddr, uint32_t paddr, size_t num_pages, uint32_t flags)
{
    return map_range(s_kernel_space, vaddr, paddr, num_pages, flags);
}

void vmm_unmap(uint32_t vaddr, size_t num_pages)
{
    unmap_range(s_kernel_space, vaddr, num_pages);
}

uint32_t vmm_virt_to_phys(uint32_t vaddr)
{
    if (!s_initialized || (vaddr & (PAGE_SIZE - 1)) != 0 ||
        vaddr >= VMM_IDENTITY_END)
        return 0;
    uint32_t directory_index = vaddr >> 22;
    uint32_t table_index = (vaddr >> PAGE_SHIFT) & (PT_ENTRIES - 1);
    if (!(s_kernel_space.directory[directory_index] & PTE_P))
        return 0;
    uint32_t entry = s_kernel_space.tables[directory_index][table_index];
    if (!(entry & PTE_P))
        return 0;
    return (entry & 0xFFFFF000u) + (vaddr & (PAGE_SIZE - 1));
}

uint32_t vmm_kernel_directory()
{
    return directory_address(s_kernel_space);
}

uint32_t vmm_create_user_directory()
{
    if (!s_initialized)
        return 0;
    for (uint32_t i = 0; i < USER_SPACE_COUNT; i++)
    {
        if (!s_user_directory_used[i])
        {
            s_user_directory_used[i] = true;
            return directory_address(s_user_spaces[i]);
        }
    }
    return 0;
}

int vmm_map_user(uint32_t directory, uint32_t vaddr, uint32_t paddr,
                 size_t num_pages, uint32_t flags)
{
    if (vaddr < VMM_USER_START || vaddr >= VMM_USER_END || !(flags & PTE_US))
        return -1;
    AddressSpace* space = user_space_for_directory(directory);
    if (!space)
        return -1;
    return map_range(*space, vaddr, paddr, num_pages, flags);
}

bool vmm_user_range_ok(uint32_t directory, uint32_t vaddr, size_t size,
                       bool write)
{
    if (size == 0)
        return true;
    if (directory == 0 || vaddr < VMM_USER_START || vaddr >= VMM_USER_END ||
        size > VMM_USER_END - vaddr)
        return false;

    while (size != 0)
    {
        uint32_t page = vaddr & ~(PAGE_SIZE - 1);
        uint32_t directory_index = page >> 22;
        uint32_t table_index = (page >> PAGE_SHIFT) & (PT_ENTRIES - 1);
        uint32_t pde =
            reinterpret_cast<volatile uint32_t*>(directory)[directory_index];
        if (!(pde & PTE_P) || !(pde & PTE_US))
            return false;
        uint32_t pte_address =
            (pde & 0xFFFFF000u) + table_index * sizeof(uint32_t);
        uint32_t pte = *reinterpret_cast<volatile uint32_t*>(pte_address);
        if (!(pte & PTE_P) || !(pte & PTE_US))
            return false;
        if (write && !(pte & PTE_RW))
            return false;
        size_t chunk = PAGE_SIZE - (vaddr & (PAGE_SIZE - 1));
        if (chunk > size)
            chunk = size;
        vaddr += static_cast<uint32_t>(chunk);
        size -= chunk;
    }
    return true;
}

}
