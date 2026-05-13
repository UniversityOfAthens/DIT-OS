#ifndef KERNEL_DEV_DRIVER_HPP
#define KERNEL_DEV_DRIVER_HPP

#include <stdint.h>
#include "dev/device.hpp"

namespace dev
{

/** Driver probe: return 0 on success, bind to device; negative on no match /
 * error */
using driver_probe_t = int (*)(Device* dev);

/** Driver operations (optional per-driver) */
struct DriverOps {
    int (*init)(Device* dev);
    void (*remove)(Device* dev);
};

struct Driver {
    const char* name;
    driver_probe_t probe;
    DriverOps ops;
};

} // namespace dev

#endif
