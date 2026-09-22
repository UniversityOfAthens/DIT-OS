// DIT-OS Kernel - Checked access to user memory (see mm/uaccess.hpp)

#include "mm/uaccess.hpp"
#include "errno_defs.hpp"
#include <stddef.h>
#include <stdint.h>

namespace mm
{

bool user_range_ok(uintptr_t addr, size_t size)
{
    if (size == 0)
        return true;
    // Written so that addr + size cannot overflow.
    return addr >= USER_SPACE_START && addr < USER_SPACE_END &&
           size <= USER_SPACE_END - addr;
}

int copy_from_user(void* dst, uintptr_t src, size_t size)
{
    if (!user_range_ok(src, size))
        return -EFAULT;
    const volatile uint8_t* s = reinterpret_cast<const volatile uint8_t*>(src);
    uint8_t* d = static_cast<uint8_t*>(dst);
    for (size_t i = 0; i < size; i++)
        d[i] = s[i];
    return 0;
}

int copy_to_user(uintptr_t dst, const void* src, size_t size)
{
    if (!user_range_ok(dst, size))
        return -EFAULT;
    volatile uint8_t* d = reinterpret_cast<volatile uint8_t*>(dst);
    const uint8_t* s = static_cast<const uint8_t*>(src);
    for (size_t i = 0; i < size; i++)
        d[i] = s[i];
    return 0;
}

int strncpy_from_user(char* dst, uintptr_t src, size_t cap)
{
    if (cap == 0)
        return -ENAMETOOLONG;
    // Check byte by byte: the string may end right before the window edge.
    for (size_t i = 0; i < cap; i++) {
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
