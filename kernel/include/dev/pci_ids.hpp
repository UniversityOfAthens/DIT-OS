#ifndef KERNEL_DEV_PCI_IDS_HPP
#define KERNEL_DEV_PCI_IDS_HPP

#include <stdint.h>

namespace dev
{

constexpr uint32_t PCI_CLASS_STORAGE = 0x010000u;
constexpr uint32_t PCI_CLASS_IDE = 0x010100u;
constexpr uint32_t PCI_CLASS_SATA = 0x010600u;
constexpr uint32_t PCI_CLASS_NVM = 0x010802u;
constexpr uint32_t PCI_CLASS_NETWORK = 0x020000u;
constexpr uint32_t PCI_CLASS_DISPLAY = 0x030000u;

constexpr uint16_t PCI_VENDOR_INTEL = 0x8086u;
constexpr uint16_t PCI_VENDOR_AMD = 0x1022u;
constexpr uint16_t PCI_VENDOR_QEMU = 0x1234u;
constexpr uint16_t PCI_VENDOR_REDHAT = 0x1AF4u;

} // namespace dev
#endif
