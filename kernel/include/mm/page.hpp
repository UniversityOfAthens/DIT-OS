#ifndef KERNEL_MM_PAGE_HPP
#define KERNEL_MM_PAGE_HPP

#include <stddef.h>

namespace mm
{

constexpr size_t PAGE_SIZE = 4096;

inline size_t pages_for_bytes(size_t bytes)
{
    return (bytes + PAGE_SIZE - 1) / PAGE_SIZE;
}

inline size_t bytes_for_pages(size_t pages)
{
    return pages * PAGE_SIZE;
}

} // namespace mm

#endif
