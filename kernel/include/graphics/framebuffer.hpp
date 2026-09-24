#ifndef KERNEL_GRAPHICS_FRAMEBUFFER_HPP
#define KERNEL_GRAPHICS_FRAMEBUFFER_HPP

#include <stdint.h>

namespace gfx
{

/** Linear framebuffer info (e.g. from Multiboot or VESA) */
struct Framebuffer
{
    void* base; /* physical or virtual base address */
    uint32_t width;
    uint32_t height;
    uint32_t pitch; /* bytes per line */
    uint8_t bpp; /* bits per pixel (16, 24, 32) */
    uint8_t red_shift;
    uint8_t red_mask;
    uint8_t green_shift;
    uint8_t green_mask;
    uint8_t blue_shift;
    uint8_t blue_mask;
};

/** Get current framebuffer (nullptr if text mode only) */
Framebuffer* framebuffer_get();

/** Set pixel at (x,y) for 32bpp; color in 0xAARRGGBB */
void framebuffer_put_pixel32(Framebuffer* fb, uint32_t x, uint32_t y,
                             uint32_t color);

} // namespace gfx

#endif
