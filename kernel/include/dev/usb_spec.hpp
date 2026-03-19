#ifndef KERNEL_DEV_USB_SPEC_HPP
#define KERNEL_DEV_USB_SPEC_HPP

#include <stdint.h>

namespace dev {
namespace usb {

/** USB specification constants (USB 1.1 / 2.0) */

constexpr uint8_t  DEVICE_DESCRIPTOR_TYPE    = 0x01;
constexpr uint8_t  CONFIG_DESCRIPTOR_TYPE   = 0x02;
constexpr uint8_t  STRING_DESCRIPTOR_TYPE   = 0x03;
constexpr uint8_t  INTERFACE_DESCRIPTOR_TYPE= 0x04;
constexpr uint8_t  ENDPOINT_DESCRIPTOR_TYPE = 0x05;

constexpr uint8_t  REQUEST_GET_DESCRIPTOR   = 0x06;
constexpr uint8_t  REQUEST_SET_ADDRESS      = 0x05;
constexpr uint8_t  REQUEST_SET_CONFIGURATION= 0x09;

constexpr uint8_t  DESC_TYPE_DEVICE        = 0x01;
constexpr uint16_t DESC_INDEX_DEVICE      = 0x00;

constexpr uint8_t  ENDPOINT_IN             = 0x80;
constexpr uint8_t  ENDPOINT_OUT            = 0x00;

constexpr uint8_t  CLASS_HID               = 0x03;
constexpr uint8_t  CLASS_MASS_STORAGE      = 0x08;
constexpr uint8_t  CLASS_HUB               = 0x09;

} // namespace usb
} // namespace dev

#endif
