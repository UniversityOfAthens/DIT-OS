// =============================================================================
// Simple filesystem reader - detect and delegate to FAT/EXT2/ISO9660
// =============================================================================

#include "../../include/fs.hpp"
#include "../../include/disk.hpp"
#include "../../include/types.hpp"

namespace boot {

// Stub: no filesystem detection yet; would read first sector and check magic
FSReader *fs_detect(u8 /*drive*/) {
    return nullptr;
}

} // namespace boot
