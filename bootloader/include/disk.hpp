#ifndef BOOTLOADER_DISK_HPP
#define BOOTLOADER_DISK_HPP

#include "types.hpp"

namespace boot {

// CHS read: drive in DL (0x80 = first HDD), buffer in ES:BX
// Returns: carry set on error
bool disk_read_chs(u8 drive, u8 head, u8 sector, u8 cylinder, u8 count, void *buffer);

// LBA read (uses extended int 13h AH=42h)
bool disk_read_lba(u8 drive, u32 lba, u16 count, void *buffer);

// Get boot drive (set by stage1)
u8 get_boot_drive();
void set_boot_drive(u8 drive);

} // namespace boot

#endif
