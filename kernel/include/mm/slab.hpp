#ifndef KERNEL_MM_SLAB_HPP
#define KERNEL_MM_SLAB_HPP

#include <stddef.h>

namespace mm
{

// Slab allocator - stub for object-sized caches
void* slab_alloc(size_t size);
void slab_free(void* ptr, size_t size);

} // namespace mm

#endif
