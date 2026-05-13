// =============================================================================
// Early paging setup - identity map low memory
// =============================================================================

#include "../include/types.hpp"

namespace boot
{

// Identity map first 4MB (call from protected mode)
void paging_identity_4mb()
{
    // Typically done in asm (CR3, page tables). Stub for C build.
}

} // namespace boot
