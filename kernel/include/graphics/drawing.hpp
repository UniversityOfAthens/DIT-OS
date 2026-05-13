#ifndef KERNEL_GRAPHICS_DRAWING_HPP
#define KERNEL_GRAPHICS_DRAWING_HPP

#include <stdint.h>
#include "graphics/framebuffer.hpp"
#include "graphics/font.hpp"
#include "graphics/color.hpp"

namespace gfx
{

/** Draw a horizontal line (x0,y) to (x1,y) inclusive, 32bpp */
void draw_hline32(Framebuffer* fb, uint32_t x0, uint32_t x1, uint32_t y,
                  uint32_t color);

/** Draw a vertical line (x,y0) to (x,y1) inclusive, 32bpp */
void draw_vline32(Framebuffer* fb, uint32_t x, uint32_t y0, uint32_t y1,
                  uint32_t color);

/** Fill rectangle [x0,y0] to [x1,y1] inclusive, 32bpp */
void draw_fill_rect32(Framebuffer* fb, uint32_t x0, uint32_t y0, uint32_t x1,
                      uint32_t y1, uint32_t color);

/** Draw a single glyph from font at (x,y), 32bpp; returns advance width */
uint32_t draw_glyph32(Framebuffer* fb, const Font* font, uint32_t code,
                      uint32_t x, uint32_t y, uint32_t fg, uint32_t bg);

} // namespace gfx

#endif
