#ifndef BOOTLOADER_BOOT_HPP
#define BOOTLOADER_BOOT_HPP

#include "types.hpp"

namespace boot
{

// Load addresses
constexpr u32 STAGE1_LOAD = 0x7C00;
constexpr u32 STAGE2_LOAD = 0x7E00;
constexpr u32 KERNEL_LOAD = 0x100000;

// Sector
constexpr u32 SECTOR_SIZE = 512;

// Sector counts
constexpr u32 STAGE2_SECTOR_COUNT = 9;
constexpr u32 KERNEL_SECTOR_COUNT = 248;

// Boot drive (set by stage1, passed in DL)
extern u8 boot_drive;

// Multiboot magic (set by bootloader before jumping to kernel)
constexpr u32 MULTIBOOT_MAGIC = 0x2BADB002;

} // namespace boot

#endif
