// Stub - early memblock allocator
#include "mm/memblock.hpp"
namespace mm
{
void* memblock_alloc(size_t)
{
    return nullptr;
}
void memblock_free(void*, size_t)
{
}
} // namespace mm
