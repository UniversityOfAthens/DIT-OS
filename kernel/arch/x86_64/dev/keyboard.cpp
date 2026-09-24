// DIT-OS Kernel - PS/2 Keyboard

#include "dev/keyboard.hpp"
#include "arch/x86_64/io.hpp"
#include <stdint.h>

namespace dev
{

static constexpr uint16_t PS2_DATA = 0x60;
static constexpr uint16_t PS2_STATUS = 0x64;
static constexpr uint8_t STATUS_OUT = 0x01;

static bool shift_down;
static bool caps_lock;

static const char scancode_ascii[128] = {
    0,   0x1B, '1',  '2', '3',  '4', '5', '6', '7', '8', '9', '0', '-',
    '=', '\b', '\t', 'q', 'w',  'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',  '\n', 0,   'a',  's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',  0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '/',  0,    '*', 0,    ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   '7', '8', '9', '-', '4', '5', '6',
    '+', '1',  '2',  '3', '0',  '.', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,    0,   0,   0,   0,   0,   0,   0,   0
};

static const char scancode_shift[128] = {
    0,   0x1B, '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   '7', '8', '9', '-', '4', '5', '6',
    '+', '1',  '2',  '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

void keyboard_init()
{
    shift_down = false;
    caps_lock = false;
}

int keyboard_has_input()
{
    return (arch::inb(PS2_STATUS) & STATUS_OUT) ? 1 : 0;
}

int keyboard_get_scancode()
{
    if (!keyboard_has_input())
        return -1;
    return arch::inb(PS2_DATA) & 0xFF;
}

int keyboard_getchar()
{
    int raw = keyboard_get_scancode();
    if (raw < 0)
        return -1;
    uint8_t sc = static_cast<uint8_t>(raw);
    if (sc & 0x80)
    {
        if (sc == 0xAA || sc == 0xB6)
            shift_down = false;
        return -1;
    }
    if (sc == 0x2A || sc == 0x36)
    {
        shift_down = true;
        return -1;
    }
    if (sc == 0x3A)
    {
        caps_lock = !caps_lock;
        return -1;
    }
    if (sc >= 128)
        return -1;
    bool use_shift = (shift_down != caps_lock);
    char c = use_shift ? scancode_shift[sc] : scancode_ascii[sc];
    return c ? (c & 0xFF) : -1;
}

} // namespace dev
