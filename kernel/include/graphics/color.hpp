#ifndef KERNEL_GRAPHICS_COLOR_HPP
#define KERNEL_GRAPHICS_COLOR_HPP

#include <stdint.h>

namespace gfx
{

/** 32-bit ARGB (8 bits each) */
struct Color32
{
    uint8_t b, g, r, a;
};

inline uint32_t color32_to_uint32(Color32 c)
{
    return (uint32_t)c.a << 24 | (uint32_t)c.r << 16 | (uint32_t)c.g << 8 | c.b;
}

inline Color32 uint32_to_color32(uint32_t v)
{
    Color32 c;
    c.b = (uint8_t)(v);
    c.g = (uint8_t)(v >> 8);
    c.r = (uint8_t)(v >> 16);
    c.a = (uint8_t)(v >> 24);
    return c;
}

/** 16-bit RGB565 */
inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)((r >> 3) << 11 | (g >> 2) << 5 | (b >> 3));
}

} // namespace gfx

#endif
