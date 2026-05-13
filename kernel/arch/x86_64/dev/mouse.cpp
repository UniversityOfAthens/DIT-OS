// =============================================================================
// DIT-OS Kernel - PS/2 Mouse (aux device on port 0x60/0x64)
// =============================================================================

#include "dev/mouse.hpp"
#include "arch/x86_64/io.hpp"
#include <stdint.h>

namespace dev
{

static constexpr uint16_t PS2_DATA = 0x60;
static constexpr uint16_t PS2_STATUS = 0x64;
static constexpr uint16_t PS2_CMD = 0x64;

static constexpr uint8_t STATUS_OUT = 0x01;
static constexpr uint8_t CMD_READ_CONFIG = 0x20;
static constexpr uint8_t CMD_WRITE_CONFIG = 0x60;
static constexpr uint8_t CMD_AUX_DISABLE = 0xA7;
static constexpr uint8_t CMD_AUX_ENABLE = 0xA8;
static constexpr uint8_t CMD_AUX_WRITE = 0xD4;

static uint8_t packet_buf[3];
static int packet_idx;
static bool enabled;

static void ps2_write(uint8_t v)
{
    while (arch::inb(PS2_STATUS) & 0x02)
        ;
    arch::outb(PS2_DATA, v);
}

static uint8_t ps2_read()
{
    while (!(arch::inb(PS2_STATUS) & STATUS_OUT))
        ;
    return arch::inb(PS2_DATA);
}

static void send_aux(uint8_t v)
{
    while (arch::inb(PS2_STATUS) & 0x02)
        ;
    arch::outb(PS2_CMD, CMD_AUX_WRITE);
    while (arch::inb(PS2_STATUS) & 0x02)
        ;
    arch::outb(PS2_DATA, v);
}

void mouse_init()
{
    packet_idx = 0;
    enabled = false;
    arch::outb(PS2_CMD, CMD_AUX_DISABLE);
    uint8_t cfg = 0;
    arch::outb(PS2_CMD, CMD_READ_CONFIG);
    cfg = ps2_read();
    cfg |= 0x02;
    arch::outb(PS2_CMD, CMD_WRITE_CONFIG);
    ps2_write(cfg);
    arch::outb(PS2_CMD, CMD_AUX_ENABLE);
    send_aux(0xF6);
    ps2_read();
    send_aux(0xF4);
    ps2_read();
    enabled = true;
}

int mouse_has_input()
{
    if (!enabled)
        return 0;
    return (arch::inb(PS2_STATUS) & STATUS_OUT) ? 1 : 0;
}

MousePacket mouse_read()
{
    MousePacket p = { 0, 0, 0, false };
    if (!mouse_has_input())
        return p;
    uint8_t b = arch::inb(PS2_DATA);
    if (packet_idx == 0 && (b & 0x08) == 0)
        return p;
    packet_buf[packet_idx++] = b;
    if (packet_idx >= 3) {
        packet_idx = 0;
        p.buttons = packet_buf[0] & 0x07;
        p.dx = static_cast<int8_t>(packet_buf[1]);
        p.dy = static_cast<int8_t>(packet_buf[2]);
        p.valid = true;
    }
    return p;
}

} // namespace dev
