#ifndef STAGE1_BOOT_HPP
#define STAGE1_BOOT_HPP

// Boot sector layout constants (for stage1 / MBR)
// Used by both stage1.asm and any C code that parses the boot sector

namespace stage1
{

constexpr unsigned BOOT_SIGNATURE_OFFSET = 510;
constexpr unsigned short BOOT_SIGNATURE = 0xAA55;

constexpr unsigned STAGE2_START_SECTOR =
    2; // sector 1 = boot sector, 2+ = stage2
constexpr unsigned STAGE2_SECTOR_COUNT = 9; // 9 * 512 = 4608 bytes for stage2
constexpr unsigned STAGE2_LOAD_SEGMENT = 0x0000;
constexpr unsigned STAGE2_LOAD_OFFSET = 0x7E00;

} // namespace stage1

#endif
