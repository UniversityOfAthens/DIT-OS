#ifndef BOOTLOADER_VIDEO_HPP
#define BOOTLOADER_VIDEO_HPP

#include "types.hpp"

namespace boot {

constexpr u32 VGA_TEXT_BASE = 0xB8000;
constexpr int VGA_COLS      = 80;
constexpr int VGA_ROWS      = 25;

enum class VGAMode : u16 {
    Text80x25 = 0x03,
    VBE       = 0x114,  // 800x600x32 or similar
};

// Set VGA mode (call in real mode)
bool video_set_mode(VGAMode mode);

// VBE: get/set linear framebuffer mode
bool video_vbe_get_info(u16 mode, u32 *width, u32 *height, u32 *bpp, u32 *pitch);
bool video_vbe_set_mode(u16 mode);

// Text cursor position
void video_set_cursor(int x, int y);
void video_get_cursor(int *x, int *y);

} // namespace boot

#endif
