#ifndef KERNEL_DEV_RTC_HPP
#define KERNEL_DEV_RTC_HPP

#include <stdint.h>

namespace dev {

struct RtcTime {
    uint8_t second, minute, hour;
    uint8_t day, month;
    uint16_t year;
};

void rtc_init();
void rtc_read(RtcTime *out);
uint8_t rtc_read_reg(uint8_t reg);

}
#endif
