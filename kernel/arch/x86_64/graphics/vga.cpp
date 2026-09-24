// DIT-OS Kernel - VGA text mode helpers

#include "graphics/vga.hpp"

namespace gfx
{

volatile uint16_t* vga_buffer()
{
    return reinterpret_cast<volatile uint16_t*>(VGA_BUFFER_PHYS);
}

void vga_clear(uint8_t attr)
{
    volatile uint16_t* vga = vga_buffer();
    const uint16_t cell = vga_cell(' ', attr);
    for (unsigned y = 0; y < VGA_HEIGHT; y++)
    {
        for (unsigned x = 0; x < VGA_WIDTH; x++)
        {
            vga[y * VGA_WIDTH + x] = cell;
        }
    }
}

void vga_putc(unsigned x, unsigned y, char c, uint8_t attr)
{
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
        return;
    volatile uint16_t* vga = vga_buffer();
    vga[y * VGA_WIDTH + x] = vga_cell(c, attr);
}

} // namespace gfx
