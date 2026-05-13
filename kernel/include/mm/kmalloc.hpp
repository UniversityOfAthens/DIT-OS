#ifndef KERNEL_MM_KMALLOC_HPP
#define KERNEL_MM_KMALLOC_HPP

#include <stddef.h>

namespace mm
{

void heap_init();
void* heap_alloc(size_t size);
void heap_free(void* ptr);

inline void* kmalloc(size_t size)
{
    return heap_alloc(size);
}
inline void kfree(void* ptr)
{
    heap_free(ptr);
}

} // namespace mm

#endif
