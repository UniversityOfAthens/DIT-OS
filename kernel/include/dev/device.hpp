#ifndef KERNEL_DEV_DEVICE_HPP
#define KERNEL_DEV_DEVICE_HPP

#include <stdint.h>

namespace dev
{

/** Base device type for device tree and driver binding */
struct Device
{
    const char* name;
    void* driver_data;
    Device* parent;
    Device* next_sibling;
};

} // namespace dev

#endif
