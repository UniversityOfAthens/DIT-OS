#ifndef KERNEL_DEV_MOUSE_HPP
#define KERNEL_DEV_MOUSE_HPP

#include <stdint.h>

namespace dev
{

struct MousePacket
{
    int8_t dx, dy;
    uint8_t buttons;
    bool valid;
};

void mouse_init();
MousePacket mouse_read();
int mouse_has_input();

} // namespace dev
#endif
