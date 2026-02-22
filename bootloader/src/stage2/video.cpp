// =============================================================================
// Video mode setup - VGA / VBE
// =============================================================================

#include "../../include/video.hpp"
#include "../../include/types.hpp"

namespace boot {

// Set mode via int 10h (call from real mode)
bool video_set_mode(VGAMode mode) {
    (void)mode;
    return true;
}

bool video_vbe_get_info(u16 /*mode*/, u32 *width, u32 *height, u32 *bpp, u32 *pitch) {
    if (width) *width = 800;
    if (height) *height = 600;
    if (bpp) *bpp = 32;
    if (pitch) *pitch = 3200;
    return true;
}

bool video_vbe_set_mode(u16 /*mode*/) {
    return true;
}

void video_set_cursor(int /*x*/, int /*y*/) {}
void video_get_cursor(int *x, int *y) {
    if (x) *x = 0;
    if (y) *y = 0;
}

} // namespace boot
