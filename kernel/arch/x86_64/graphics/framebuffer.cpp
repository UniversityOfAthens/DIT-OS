// DIT-OS Kernel - Minimal linear framebuffer helpers

#include "graphics/framebuffer.hpp"

namespace gfx
{

static Framebuffer g_fb = { nullptr, /* base */
                            0, /* width */
                            0, /* height */
                            0, /* pitch */
                            0, /* bpp */
                            0,       0, 0, 0, 0, 0 };

Framebuffer* framebuffer_get()
{
    if (!g_fb.base || !g_fb.width || !g_fb.height)
        return nullptr;
    return &g_fb;
}

void framebuffer_put_pixel32(Framebuffer* fb, uint32_t x, uint32_t y,
                             uint32_t color)
{
    if (!fb || !fb->base)
        return;
    if (fb->bpp != 32)
        return;
    if (x >= fb->width || y >= fb->height)
        return;

    uint8_t* row = reinterpret_cast<uint8_t*>(fb->base) + (y * fb->pitch);
    uint32_t* pixel = reinterpret_cast<uint32_t*>(row + (x * 4));
    *pixel = color;
}

} // namespace gfx
