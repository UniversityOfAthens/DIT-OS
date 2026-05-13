#include "dev/rtc.hpp"
#include "arch/x86_64/io.hpp"

namespace dev
{

static constexpr uint16_t RTC_INDEX = 0x70;
static constexpr uint16_t RTC_DATA = 0x71;

static constexpr uint8_t RTC_SEC = 0, RTC_MIN = 2, RTC_HOUR = 4;
static constexpr uint8_t RTC_DAY = 7, RTC_MONTH = 8, RTC_YEAR = 9;
static constexpr uint8_t RTC_B = 0x0B;

static uint8_t bcd_to_bin(uint8_t bcd)
{
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

void rtc_init()
{
    arch::outb(RTC_INDEX, RTC_B);
    uint8_t b = arch::inb(RTC_DATA);
    arch::outb(RTC_INDEX, RTC_B);
    arch::outb(RTC_DATA, b | 0x02);
}

uint8_t rtc_read_reg(uint8_t reg)
{
    arch::outb(RTC_INDEX, reg);
    return arch::inb(RTC_DATA);
}

void rtc_read(RtcTime* out)
{
    if (!out)
        return;
    while (rtc_read_reg(0x0A) & 0x80) {
    }
    out->second = bcd_to_bin(rtc_read_reg(RTC_SEC));
    out->minute = bcd_to_bin(rtc_read_reg(RTC_MIN));
    out->hour = bcd_to_bin(rtc_read_reg(RTC_HOUR));
    out->day = bcd_to_bin(rtc_read_reg(RTC_DAY));
    out->month = bcd_to_bin(rtc_read_reg(RTC_MONTH));
    out->year = (uint16_t)bcd_to_bin(rtc_read_reg(RTC_YEAR)) + 2000;
}

} // namespace dev
