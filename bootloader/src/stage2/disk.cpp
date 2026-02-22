// =============================================================================
// Disk I/O - C++ wrapper
// =============================================================================

#include "../../include/disk.hpp"
#include "../../include/boot.hpp"

namespace boot {

static u8 s_boot_drive = 0x80;

u8 get_boot_drive() { return s_boot_drive; }
void set_boot_drive(u8 drive) { s_boot_drive = drive; }

// Real disk I/O is done from assembly (int 13h). These wrappers are for
// future use when calling from C in a realmode shim or via V86.
bool disk_read_chs(u8 /*drive*/, u8 /*head*/, u8 /*sector*/, u8 /*cylinder*/, u8 /*count*/, void * /*buffer*/) {
    return false;
}

bool disk_read_lba(u8 /*drive*/, u32 /*lba*/, u16 /*count*/, void * /*buffer*/) {
    return false;
}

} // namespace boot
