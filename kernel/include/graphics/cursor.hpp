#ifndef KERNEL_GRAPHICS_CURSOR_HPP
#define KERNEL_GRAPHICS_CURSOR_HPP

#include <stdint.h>

namespace gfx {

/** Cursor position (text or pixel) */
struct Cursor {
    uint32_t x;
    uint32_t y;
};

/** Show/hide hardware text cursor (VGA CRTC) */
void cursor_enable(int visible);

/** Set hardware text cursor position (column, row) */
void cursor_set(unsigned col, unsigned row);

/** Get current cursor position */
void cursor_get(unsigned* col, unsigned* row);

} // namespace gfx

#endif
