#ifndef KERNEL_DEV_BUS_HPP
#define KERNEL_DEV_BUS_HPP

#include <stdint.h>
#include "dev/device.hpp"
#include "dev/driver.hpp"

namespace dev
{

/** Bus type for enumerating and attaching drivers to devices */
struct Bus
{
    const char* name;
    int (*enumerate)(Bus* bus);
    int (*attach)(Device* dev, Driver* drv);
};

} // namespace dev

#endif
