// DIT-OS Kernel - Checked access to user memory (see mm/uaccess.hpp)

#include "mm/uaccess.hpp"
#include "arch/x86_64/control.hpp"
#include "mm/vmm.hpp"
#include "errno_defs.hpp"
#include <stddef.h>
#include <stdint.h>

namespace mm
{

bool user_range_ok(uintptr_t addr, size_t size)
{
    if (size == 0)
        return true;
    return mm::vmm_user_range_ok(arch::read_cr3(), static_cast<uint32_t>(addr),
                                 size, false);
}

int copy_from_user(void* dst, uintptr_t src, size_t size)
{
    if (!mm::vmm_user_range_ok(arch::read_cr3(), static_cast<uint32_t>(src),
                               size, false))
        return -EFAULT;
    const volatile uint8_t* s = reinterpret_cast<const volatile uint8_t*>(src);
    uint8_t* d = static_cast<uint8_t*>(dst);
    for (size_t i = 0; i < size; i++)
        d[i] = s[i];
    return 0;
}

int copy_to_user(uintptr_t dst, const void* src, size_t size)
{
    /* Validate the range read-only: fork() clears PTE_RW on every present
     * user page, so a page the process has not written since fork() is not
     * writable yet (M39). Writability is restored page by page below by
     * vmm_write_user()/page_for_write(); storing through the user VA would
     * write-fault in kernel mode, and the fault handler only fixes up user
     * faults. A short copy can only happen when a frame cannot be acquired
     * mid-way; the caller then sees -EFAULT on a partially written buffer. */
    if (!mm::vmm_user_range_ok(arch::read_cr3(), static_cast<uint32_t>(dst),
                               size, false))
        return -EFAULT;
    if (mm::vmm_write_user(arch::read_cr3(), static_cast<uint32_t>(dst), src,
                           static_cast<uint32_t>(size)) !=
        static_cast<uint32_t>(size))
        return -EFAULT;
    return 0;
}

int strncpy_from_user(char* dst, uintptr_t src, size_t cap)
{
    if (cap == 0)
        return -ENAMETOOLONG;
    // Check byte by byte: the string may end right before the window edge.
    for (size_t i = 0; i < cap; i++)
    {
        if (!user_range_ok(src + i, 1))
            return -EFAULT;
        char c = *reinterpret_cast<const volatile char*>(src + i);
        dst[i] = c;
        if (c == '\0')
            return static_cast<int>(i);
    }
    dst[cap - 1] = '\0';
    return -ENAMETOOLONG;
}

} // namespace mm
