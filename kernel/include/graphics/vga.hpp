#ifndef KERNEL_GRAPHICS_VGA_HPP
#define KERNEL_GRAPHICS_VGA_HPP

#include <stdint.h>

namespace gfx {

/** VGA text mode: 80x25, 16 colors, buffer at 0xB8000 */
constexpr uint16_t VGA_WIDTH  = 80;
constexpr uint16_t VGA_HEIGHT = 25;
constexpr uintptr_t VGA_BUFFER_PHYS = 0xB8000;

/** 16-color VGA palette (index 0-15) */
enum VgaColor : uint8_t {
    VGA_BLACK = 0,
    VGA_BLUE,
    VGA_GREEN,
    VGA_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_BROWN,
    VGA_LIGHT_GREY,
    VGA_DARK_GREY,
    VGA_LIGHT_BLUE,
    VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN,
    VGA_LIGHT_RED,
    VGA_LIGHT_MAGENTA,
    VGA_YELLOW,
    VGA_WHITE,
};

/** Build attribute byte: fg | (bg << 4) */
inline uint8_t vga_attr(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

/** Build 16-bit cell: char | (attr << 8) */
inline uint16_t vga_cell(char c, uint8_t attr) {
    return (uint16_t)(uint8_t)c | ((uint16_t)attr << 8);
}

/** Get pointer to VGA text buffer (caller must map 0xB8000 if paging) */
volatile uint16_t* vga_buffer();

/** Clear entire screen with attribute */
void vga_clear(uint8_t attr);

/** Put character at (x,y) with attribute */
void vga_putc(unsigned x, unsigned y, char c, uint8_t attr);

} // namespace gfx

#endif
