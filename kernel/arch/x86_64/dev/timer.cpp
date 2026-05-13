// =============================================================================
// DIT-OS Kernel - PIT (Programmable Interval Timer) channel 0
// =============================================================================

#include "dev/timer.hpp"
#include "arch/x86_64/io.hpp"
#include <stdint.h>

namespace dev
{

static constexpr uint16_t PIT_CH0 = 0x40;
static constexpr uint16_t PIT_CMD = 0x43;
static constexpr uint8_t PIT_CH0_SQUARE = 0x36;
static constexpr uint32_t PIT_BASE_HZ = 1193182;

static volatile uint32_t s_ticks;

void timer_init()
{
    s_ticks = 0;
    timer_set_hz(100);
}

void timer_set_hz(uint32_t hz)
{
    if (hz == 0)
        hz = 1;
    uint32_t div = PIT_BASE_HZ / hz;
    if (div > 65535)
        div = 65535;
    arch::outb(PIT_CMD, PIT_CH0_SQUARE);
    arch::outb(PIT_CH0, div & 0xFF);
    arch::outb(PIT_CH0, (div >> 8) & 0xFF);
}

void timer_tick()
{
    s_ticks++;
}

uint32_t timer_ticks()
{
    return s_ticks;
}

void timer_wait_ticks(uint32_t ticks)
{
    uint32_t end = s_ticks + ticks;
    while (s_ticks < end)
        ;
}

void timer_wait_ms(uint32_t ms)
{
    uint32_t t = (ms * 100) / 1000;
    if (t == 0)
        t = 1;
    timer_wait_ticks(t);
}

} // namespace dev
