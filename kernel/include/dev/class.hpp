#ifndef KERNEL_DEV_CLASS_HPP
#define KERNEL_DEV_CLASS_HPP

#include <stdint.h>

namespace dev {

/** Device class for grouping devices (block, char, net, etc.) */
enum class DeviceClass : uint8_t {
    Block,
    Char,
    Net,
    Display,
    Input,
    Other,
};

} // namespace dev

#endif
