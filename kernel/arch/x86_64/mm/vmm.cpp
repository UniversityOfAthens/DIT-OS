// DIT-OS Kernel - Virtual memory manager
//
// The kernel owns one identity-mapped address space covering
// [0, VMM_IDENTITY_END), so every physical frame below that limit can be read
// and written in place by the kernel: page tables, user page contents and
// copy-on-write duplicates all live there.
//
// Each user process gets a page directory plus one page table per 4 MiB of
// the user window, statically allocated. User pages are anonymous: touching
// a page the process owns (its break, its mappings, its stack) allocates a
// zeroed frame from the user frame pool, and touching anything else faults.
// The program image is mapped in full up front, so .bss needs no faulting.
//
// Frames are reference counted. fork() clones a space copy-on-write: both
// sides start out pointing at the same frames with the write bit cleared, and
// the first write to a shared page duplicates it (see vmm_handle_fault).

#include "mm/vmm.hpp"
#include "arch/x86_64/control.hpp"
#include "arch/x86_64/idt.hpp"
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
using arch::PTE_RW;
using arch::PTE_US;
using arch::PT_ENTRIES;

constexpr uint32_t PTE_ADDRESS_MASK = 0xFFFFF000u;
constexpr uint32_t KERNEL_PDE_COUNT = VMM_IDENTITY_END >> 22;
constexpr uint32_t USER_PDE_COUNT = (VMM_USER_END - VMM_USER_START) >> 22;
constexpr uint32_t USER_FRAME_COUNT =
    (VMM_FRAME_END - VMM_FRAME_START) / PAGE_SIZE;
constexpr uint32_t USER_IMAGE_PAGES = USER_IMAGE_SIZE / PAGE_SIZE;

static_assert(USER_PDE_COUNT == 3, "user window spans three 4 MiB chunks");
static_assert(USER_FRAME_COUNT < 0x10000, "frame refcounts are 16 bit");
static_assert(USER_STACK_TOP <= USER_BRK_START && USER_BRK_START < VMM_USER_END,
              "user regions must not overlap");
static_assert(VMM_FRAME_END <= VMM_USER_START,
              "the frame pool must stay below the user window");

/* Page directory and tables of one user space. Table 0 repeats the kernel's
 * identity mapping without the user bit, because a task runs on its own page
 * directory while still executing kernel code; tables 1..3 hold the user
 * window and are the only ones a user page fault can reach. */
struct alignas(PAGE_SIZE) AddressSpace
{
    uint32_t directory[PD_ENTRIES];
    uint32_t tables[KERNEL_PDE_COUNT + USER_PDE_COUNT][PT_ENTRIES];
};

static_assert(sizeof(AddressSpace) ==
                  (1 + KERNEL_PDE_COUNT + USER_PDE_COUNT) * PAGE_SIZE,
              "address space must be a whole number of pages");

/* Index of the first table in AddressSpace::tables that holds user pages. */
constexpr uint32_t USER_FIRST_TABLE = KERNEL_PDE_COUNT;

struct KernelSpace
{
    uint32_t directory[PD_ENTRIES];
    uint32_t tables[KERNEL_PDE_COUNT][PT_ENTRIES];
};

static_assert(sizeof(KernelSpace) == (1 + KERNEL_PDE_COUNT) * PAGE_SIZE,
              "kernel space must be a whole number of pages");

/* Per-space bookkeeping, kept out of AddressSpace so the page tables stay
 * page aligned. */
struct SpaceInfo
{
    bool used;
    uint32_t brk; /* current program break */
    uint32_t brk_end; /* highest address brk() may return */
    uint32_t mmap_next; /* first address past the last mapping */
    uint32_t stack_lo; /* this process' user stack */
    uint32_t stack_hi;
};

static KernelSpace s_kernel_space;
static AddressSpace s_user_spaces[USER_SPACE_COUNT];
static SpaceInfo s_info[USER_SPACE_COUNT];
static uint16_t s_frame_refs[USER_FRAME_COUNT];
static bool s_initialized;

void zero_pages(void* address, uint32_t pages)
{
    uint8_t* p = static_cast<uint8_t*>(address);
    for (uint32_t i = 0; i < pages * PAGE_SIZE; i++)
        p[i] = 0;
}

void copy_bytes(void* dst, const void* src, uint32_t size)
{
    uint8_t* d = static_cast<uint8_t*>(dst);
    const uint8_t* s = static_cast<const uint8_t*>(src);
    for (uint32_t i = 0; i < size; i++)
        d[i] = s[i];
}

bool frame_in_pool(uint32_t paddr)
{
    return paddr >= VMM_FRAME_START && paddr < VMM_FRAME_END;
}

/* Take a free frame from the pool and zero it. Returns 0 when the pool is
 * exhausted. The frame is inside the kernel's identity mapping, so it can be
 * filled in before any page table entry refers to it. */
uint32_t frame_acquire()
{
    if (!s_initialized)
        return 0;
    for (uint32_t i = 0; i < USER_FRAME_COUNT; i++)
    {
        if (s_frame_refs[i] != 0)
            continue;
        s_frame_refs[i] = 1;
        uint32_t paddr = VMM_FRAME_START + i * PAGE_SIZE;
        zero_pages(reinterpret_cast<void*>(paddr), 1);
        return paddr;
    }
    return 0;
}

void frame_ref(uint32_t paddr)
{
    if (frame_in_pool(paddr))
        s_frame_refs[(paddr - VMM_FRAME_START) / PAGE_SIZE]++;
}

void frame_unref(uint32_t paddr)
{
    if (!frame_in_pool(paddr))
        return;
    uint32_t i = (paddr - VMM_FRAME_START) / PAGE_SIZE;
    if (s_frame_refs[i] != 0)
        s_frame_refs[i]--;
}

AddressSpace* space_of(uint32_t directory)
{
    if (!s_initialized || directory == 0)
        return nullptr;
    for (uint32_t i = 0; i < USER_SPACE_COUNT; i++)
    {
        if (s_info[i].used &&
            directory == static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
                             s_user_spaces[i].directory)))
            return &s_user_spaces[i];
    }
    return nullptr;
}

SpaceInfo* info_of(AddressSpace* space)
{
    uint32_t index = static_cast<uint32_t>(space - s_user_spaces);
    return &s_info[index];
}

/* Page table entry for @vaddr, or nullptr when the address is outside the
 * window or no table is installed for it. */
uint32_t* pte_for(AddressSpace& space, uint32_t vaddr)
{
    uint32_t page = vaddr & ~(PAGE_SIZE - 1);
    if (page < VMM_USER_START || page >= VMM_USER_END)
        return nullptr;
    uint32_t pde = space.directory[page >> 22];
    if (!(pde & PTE_P))
        return nullptr;
    uint32_t* table = reinterpret_cast<uint32_t*>(pde & PTE_ADDRESS_MASK);
    return &table[(page >> PAGE_SHIFT) & (PT_ENTRIES - 1)];
}

/* A page the process may fault in: part of its break, its mappings or its
 * stack. The image window is mapped up front and is deliberately excluded. */
bool space_owns(const SpaceInfo& info, uint32_t page)
{
    if (page >= USER_BRK_START && page < info.brk)
        return true;
    if (page >= USER_MMAP_START && page < info.mmap_next)
        return true;
    if (page >= info.stack_lo && page < info.stack_hi)
        return true;
    return false;
}

/* Give a shared page its own copy and restore the write bit. */
int break_cow(uint32_t page, uint32_t* pte)
{
    uint32_t shared = *pte & PTE_ADDRESS_MASK;
    uint32_t paddr = frame_acquire();
    if (!paddr)
        return -1;
    copy_bytes(reinterpret_cast<void*>(paddr), reinterpret_cast<void*>(shared),
               PAGE_SIZE);
    *pte = paddr | PTE_P | PTE_RW | PTE_US;
    frame_unref(shared);
    /* The read-only translation is still cached; drop it or the write that
     * faulted here would fault again. */
    arch::tlb_invalidate(page);
    return 0;
}

/* Page table entry for @page, made present and writable, allocating or
 * un-sharing as needed. @force maps the page even when the process would not
 * be allowed to fault it in (used to load a program image). Returns the frame
 * address, or 0. */
uint32_t page_for_write(AddressSpace& space, const SpaceInfo& info,
                        uint32_t page, bool force)
{
    uint32_t* pte = pte_for(space, page);
    if (!pte)
        return 0;
    if (!(*pte & PTE_P))
    {
        if (!force && !space_owns(info, page))
            return 0;
        uint32_t paddr = frame_acquire();
        if (!paddr)
            return 0;
        *pte = paddr | PTE_P | PTE_RW | PTE_US;
        return paddr;
    }
    if (!(*pte & PTE_RW))
    {
        if (break_cow(page, pte) != 0)
            return 0;
    }
    return *pte & PTE_ADDRESS_MASK;
}

void unmap_pages(AddressSpace& space, uint32_t vaddr, uint32_t pages)
{
    for (uint32_t i = 0; i < pages; i++)
    {
        uint32_t page = (vaddr + i * PAGE_SIZE) & ~(PAGE_SIZE - 1);
        uint32_t* pte = pte_for(space, page);
        if (!pte || !(*pte & PTE_P))
            continue;
        frame_unref(*pte & PTE_ADDRESS_MASK);
        *pte = 0;
        arch::tlb_invalidate(page);
    }
}

void map_identity(uint32_t* directory, uint32_t* tables, uint32_t table_count)
{
    for (uint32_t i = 0; i < PD_ENTRIES; i++)
        directory[i] = 0;
    for (uint32_t t = 0; t < table_count; t++)
    {
        uint32_t* table = tables + t * PT_ENTRIES;
        for (uint32_t e = 0; e < PT_ENTRIES; e++)
            table[e] = ((t * PT_ENTRIES + e) << PAGE_SHIFT) | PTE_P | PTE_RW;
        directory[t] = reinterpret_cast<uint32_t>(table) | PDE_KERNEL;
    }
}

} // namespace

void vmm_init()
{
    map_identity(s_kernel_space.directory, &s_kernel_space.tables[0][0],
                 KERNEL_PDE_COUNT);

    for (uint32_t i = 0; i < USER_SPACE_COUNT; i++)
    {
        AddressSpace& space = s_user_spaces[i];
        for (uint32_t e = 0; e < PD_ENTRIES; e++)
            space.directory[e] = 0;

        /* Table 0 keeps the kernel's identity mapping (supervisor only), then
         * one table per 4 MiB of the user window, reachable from user mode.
         * The tables themselves are inside the kernel image, so the kernel
         * reaches them through their address instead of through a PDE. */
        for (uint32_t e = 0; e < PT_ENTRIES; e++)
        {
            space.tables[0][e] = (e << PAGE_SHIFT) | PTE_P | PTE_RW;
            space.directory[0] =
                reinterpret_cast<uint32_t>(&space.tables[0][0]) | arch::PTE_P |
                arch::PTE_RW;
        }
        for (uint32_t t = 0; t < USER_PDE_COUNT; t++)
        {
            for (uint32_t e = 0; e < PT_ENTRIES; e++)
                space.tables[USER_FIRST_TABLE + t][e] = 0;
            space.directory[(VMM_USER_START >> 22) + t] =
                reinterpret_cast<uint32_t>(
                    &space.tables[USER_FIRST_TABLE + t][0]) |
                arch::PTE_P | arch::PTE_RW | arch::PTE_US;
        }

        s_info[i].used = false;
    }

    /* The image window is copied to VMM_USER_START by init_user_image() before
     * any address space exists, and every space maps it, so it is permanent
     * and outside the frame pool. */
    s_initialized = true;
}

uint32_t vmm_kernel_directory()
{
    return static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(s_kernel_space.directory));
}

uint32_t vmm_create_user_directory(bool with_image)
{
    if (!s_initialized)
        return 0;
    for (uint32_t i = 0; i < USER_SPACE_COUNT; i++)
    {
        if (s_info[i].used)
            continue;

        AddressSpace& space = s_user_spaces[i];
        SpaceInfo& info = s_info[i];

        info.used = true;
        info.brk = USER_BRK_START;
        info.brk_end = USER_BRK_END;
        info.mmap_next = USER_MMAP_START;
        info.stack_hi = USER_STACK_TOP;
        info.stack_lo = USER_STACK_TOP - USER_STACK_SIZE;

        /* The image is shared by every space that runs the boot program; map
         * it present and take a reference per space so the last one out keeps
         * it alive. execve() passes false and loads its own image. */
        for (uint32_t p = 0; with_image && p < USER_IMAGE_PAGES; p++)
        {
            uint32_t vaddr = USER_IMAGE_BASE + p * PAGE_SIZE;
            uint32_t* pte = pte_for(space, vaddr);
            *pte = vaddr | PTE_P | PTE_RW | PTE_US;
            frame_ref(vaddr);
        }
        return static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(space.directory));
    }
    return 0;
}

void vmm_destroy_user_directory(uint32_t directory)
{
    AddressSpace* space = space_of(directory);
    if (!space)
        return;
    for (uint32_t page = VMM_USER_START; page < VMM_USER_END; page += PAGE_SIZE)
    {
        uint32_t* pte = pte_for(*space, page);
        if (!pte || !(*pte & PTE_P))
            continue;
        frame_unref(*pte & PTE_ADDRESS_MASK);
        *pte = 0;
    }
    SpaceInfo* info = info_of(space);
    info->used = false;
    info->brk = USER_BRK_START;
    info->brk_end = USER_BRK_END;
    info->mmap_next = USER_MMAP_START;
    info->stack_lo = 0;
    info->stack_hi = 0;
    /* The kernel mapping and the page tables are static and stay valid. */
}

uint32_t vmm_clone_user_directory(uint32_t src_directory)
{
    AddressSpace* src = space_of(src_directory);
    if (!src)
        return 0;
    SpaceInfo* src_info = info_of(src);

    uint32_t dst_directory = vmm_create_user_directory(true);
    if (!dst_directory)
        return 0;
    AddressSpace* dst = space_of(dst_directory);
    if (!dst)
        return 0;
    SpaceInfo* dst_info = info_of(dst);

    /* Replace the fresh space's heap with the source's; the image mappings it
     * already has are the same physical pages, so they are kept. */
    dst_info->brk = src_info->brk;
    dst_info->brk_end = src_info->brk_end;
    dst_info->mmap_next = src_info->mmap_next;

    for (uint32_t page = VMM_USER_START; page < VMM_USER_END; page += PAGE_SIZE)
    {
        uint32_t* src_pte = pte_for(*src, page);
        if (!src_pte || !(*src_pte & PTE_P))
            continue;
        uint32_t paddr = *src_pte & PTE_ADDRESS_MASK;
        /* Copy-on-write: both sides keep the page present and readable but
         * lose the write bit, so the first write faults and duplicates it. */
        *src_pte = paddr | PTE_P | PTE_US;
        uint32_t* dst_pte = pte_for(*dst, page);
        /* The new space is created with the image pre-mapped, so dst_pte can
         * already be present for a code-image page the source has since made
         * private. Account for the mapping we are about to replace instead of
         * assuming "present" means "already shares paddr": skipping the
         * reference let the child's exit free a frame the parent still mapped
         * (M42). */
        uint32_t previous = (*dst_pte & PTE_P) ? (*dst_pte & PTE_ADDRESS_MASK) :
                                                 0;
        if (previous != paddr)
        {
            if (previous)
                frame_unref(previous);
            frame_ref(paddr);
        }
        *dst_pte = paddr | PTE_P | PTE_US;
    }
    /* The source is the running space; its cached translations still allow
     * writes, so drop them. */
    arch::tlb_flush();
    return dst_directory;
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
            (pde & PTE_ADDRESS_MASK) + table_index * sizeof(uint32_t);
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

int vmm_handle_fault(uint32_t directory, uint32_t vaddr, uint32_t error)
{
    AddressSpace* space = space_of(directory);
    if (!space)
        return -1;
    uint32_t page = vaddr & ~(PAGE_SIZE - 1);
    if (page < VMM_USER_START || page >= VMM_USER_END)
        return -1;
    uint32_t* pte = pte_for(*space, page);
    if (!pte)
        return -1;

    if (*pte & PTE_P)
    {
        /* Present but not writable is exactly what fork() leaves behind. */
        if (!(error & arch::PF_WRITE) || (*pte & PTE_RW))
            return -1;
        return break_cow(page, pte);
    }

    if (!space_owns(*info_of(space), page))
        return -1;
    uint32_t paddr = frame_acquire();
    if (!paddr)
        return -1;
    *pte = paddr | PTE_P | PTE_RW | PTE_US;
    return 0;
}

uint32_t vmm_write_user(uint32_t directory, uint32_t vaddr, const void* src,
                        uint32_t size)
{
    AddressSpace* space = space_of(directory);
    if (!space || size == 0)
        return 0;
    if (vaddr < VMM_USER_START || vaddr >= VMM_USER_END)
        return 0;
    if (size > VMM_USER_END - vaddr)
        return 0;

    const uint8_t* in = static_cast<const uint8_t*>(src);
    const SpaceInfo& info = *info_of(space);
    uint32_t done = 0;
    while (done < size)
    {
        uint32_t at = vaddr + done;
        uint32_t page = at & ~(PAGE_SIZE - 1);
        uint32_t offset = at - page;
        uint32_t chunk = PAGE_SIZE - offset;
        if (chunk > size - done)
            chunk = size - done;
        uint32_t paddr = page_for_write(*space, info, page, false);
        if (!paddr)
            return done;
        copy_bytes(reinterpret_cast<void*>(paddr + offset), in + done, chunk);
        done += chunk;
    }
    return done;
}

uint32_t vmm_map_user_page(uint32_t directory, uint32_t vaddr)
{
    AddressSpace* space = space_of(directory);
    if (!space)
        return 0;
    uint32_t page = vaddr & ~(PAGE_SIZE - 1);
    if (page < VMM_USER_START || page >= VMM_USER_END)
        return 0;
    return page_for_write(*space, *info_of(space), page, true);
}

uint32_t vmm_brk(uint32_t directory, uint32_t addr)
{
    AddressSpace* space = space_of(directory);
    if (!space)
        return 0;
    SpaceInfo& info = *info_of(space);
    if (addr == 0)
        return info.brk;
    if (addr < USER_BRK_START || addr > info.brk_end)
        return info.brk;
    if (addr < info.brk)
    {
        /* Only pages entirely above the new break are released, so the page
         * straddling it keeps its contents. */
        uint32_t first = (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        if (first < info.brk)
            unmap_pages(*space, first, (info.brk - first) / PAGE_SIZE);
    }
    info.brk = addr;
    return info.brk;
}

uint32_t vmm_mmap(uint32_t directory, uint32_t length)
{
    AddressSpace* space = space_of(directory);
    if (!space || length == 0)
        return 0;
    if (length > VMM_USER_END - VMM_USER_START)
        return 0;
    SpaceInfo& info = *info_of(space);
    uint32_t pages = (length + PAGE_SIZE - 1) / PAGE_SIZE;
    if (pages > (VMM_USER_END - info.mmap_next) / PAGE_SIZE)
        return 0;
    uint32_t addr = info.mmap_next;
    info.mmap_next = addr + pages * PAGE_SIZE;
    return addr;
}

int vmm_munmap(uint32_t directory, uint32_t addr, uint32_t length)
{
    AddressSpace* space = space_of(directory);
    if (!space || length == 0)
        return -1;
    if (addr < VMM_USER_START || addr >= VMM_USER_END)
        return -1;
    if (addr & (PAGE_SIZE - 1))
        return -1;
    if (length > VMM_USER_END - addr)
        return -1;

    SpaceInfo& info = *info_of(space);
    uint32_t pages = length / PAGE_SIZE;
    uint32_t end = addr + pages * PAGE_SIZE;
    unmap_pages(*space, addr, pages);

    /* Releasing the newest region gives its addresses back. */
    if (end == info.mmap_next)
        info.mmap_next = addr;
    if (end == info.brk && addr >= USER_BRK_START)
        info.brk = addr;
    return 0;
}

} // namespace mm
