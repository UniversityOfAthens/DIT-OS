#ifndef KERNEL_DEV_PCI_HPP
#define KERNEL_DEV_PCI_HPP

#include <stdint.h>

namespace dev
{

struct PciDev
{
    uint8_t bus, dev, func;
    uint16_t vendor_id, device_id;
    uint8_t class_code, subclass, prog_if;
    uint8_t header_type;
    uint8_t bist, latency, cache_line;
    uint32_t bar[6];
};

uint32_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg);
void pci_write_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg,
                      uint32_t value);
void pci_read_dev(PciDev* out, uint8_t bus, uint8_t dev, uint8_t func);
void pci_scan(void (*cb)(uint8_t bus, uint8_t dev, uint8_t func, void* ctx),
              void* ctx);
int pci_find_class(uint32_t class_mask, PciDev* out);

} // namespace dev
#endif
