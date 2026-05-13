#ifndef KERNEL_DEV_KEYBOARD_HPP
#define KERNEL_DEV_KEYBOARD_HPP

#include <stdint.h>

namespace dev
{

void keyboard_init();
int keyboard_get_scancode();
int keyboard_getchar();
int keyboard_has_input();

} // namespace dev

#endif
