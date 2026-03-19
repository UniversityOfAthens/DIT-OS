#ifndef KERNEL_GRAPHICS_FONT_HPP
#define KERNEL_GRAPHICS_FONT_HPP

#include <stdint.h>

namespace gfx {

/** Fixed-width bitmap font (e.g. 8x16) */
struct Font {
    const uint8_t* data;   /* glyph bitmaps, row-major per glyph */
    uint8_t        width;  /* glyph width in pixels */
    uint8_t        height; /* glyph height in pixels */
    uint8_t        stride; /* bytes per glyph row (e.g. (width+7)/8) */
    uint32_t       first;  /* first character code (e.g. 0) */
    uint32_t       count;  /* number of glyphs */
};

/** Get default kernel font (if any) */
const Font* font_default();

/** Measure one character in pixels */
void font_measure(const Font* f, uint32_t code, uint8_t* out_w, uint8_t* out_h);

} // namespace gfx

#endif
