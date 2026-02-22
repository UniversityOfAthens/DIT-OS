// Stub - slab allocator
#include "mm/slab.hpp"
namespace mm {
void *slab_alloc(size_t) { return nullptr; }
void slab_free(void *, size_t) {}
} // namespace mm
