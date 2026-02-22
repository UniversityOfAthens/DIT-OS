#ifndef KERNEL_DEV_TIMER_HPP
#define KERNEL_DEV_TIMER_HPP

#include <stdint.h>

namespace dev {

void timer_init();
void timer_set_hz(uint32_t hz);
void timer_tick();
uint32_t timer_ticks();
void timer_wait_ticks(uint32_t ticks);
void timer_wait_ms(uint32_t ms);

} // namespace dev

#endif
