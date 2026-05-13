#ifndef KERNEL_MM_MEMBLOCK_HPP
#define KERNEL_MM_MEMBLOCK_HPP

#include <stdint.h>
#include <stddef.h>

namespace mm
{

// Early memory allocator - stub
void* memblock_alloc(size_t size);
void memblock_free(void* ptr, size_t size);

} // namespace mm

#endif
