// DIT-OS Kernel - PCI bus enumeration (config space 0xCF8 / 0xCFC)

#include "dev/pci.hpp"
#include "dev/pci_ids.hpp"
#include "arch/x86_64/io.hpp"
#include <stdint.h>

namespace dev
{

static constexpr uint16_t PCI_CONFIG_ADDR = 0xCF8;
static constexpr uint16_t PCI_CONFIG_DATA = 0xCFC;

static uint32_t make_addr(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg)
{
    return 0x80000000u | (uint32_t)bus << 16 | (uint32_t)dev << 11 |
           (uint32_t)func << 8 | (reg & 0xFCu);
}

uint32_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg)
{
    arch::outl(PCI_CONFIG_ADDR, make_addr(bus, dev, func, reg));
    return arch::inl(PCI_CONFIG_DATA);
}

void pci_write_config(uint8_t bus, uint8_t dev, uint8_t func, uint8_t reg,
                      uint32_t value)
{
    arch::outl(PCI_CONFIG_ADDR, make_addr(bus, dev, func, reg));
    arch::outl(PCI_CONFIG_DATA, value);
}

void pci_read_dev(PciDev* out, uint8_t bus, uint8_t dev, uint8_t func)
{
    if (!out)
        return;
    out->bus = bus;
    out->dev = dev;
    out->func = func;
    uint32_t v = pci_read_config(bus, dev, func, 0x00);
    out->vendor_id = (uint16_t)(v & 0xFFFFu);
    out->device_id = (uint16_t)(v >> 16);
    v = pci_read_config(bus, dev, func, 0x08);
    out->class_code = (uint8_t)(v >> 24);
    out->subclass = (uint8_t)(v >> 16);
    out->prog_if = (uint8_t)(v >> 8);
    out->header_type =
        (uint8_t)((pci_read_config(bus, dev, func, 0x0C) >> 16) & 0xFFu);
    v = pci_read_config(bus, dev, func, 0x0C);
    out->bist = (uint8_t)(v >> 24);
    out->latency = (uint8_t)(v >> 8);
    out->cache_line = (uint8_t)(v & 0xFFu);
    for (int i = 0; i < 6; i++)
        out->bar[i] = pci_read_config(bus, dev, func, 0x10 + i * 4);
}

void pci_scan(void (*cb)(uint8_t bus, uint8_t dev, uint8_t func, void* ctx),
              void* ctx)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            uint32_t v = pci_read_config((uint8_t)bus, dev, 0, 0);
            if ((v & 0xFFFFu) == 0xFFFFu)
                continue;
            for (uint8_t func = 0; func < 8; func++) {
                v = pci_read_config((uint8_t)bus, dev, func, 0);
                if ((v & 0xFFFFu) == 0xFFFFu)
                    continue;
                if (cb)
                    cb((uint8_t)bus, dev, func, ctx);
            }
        }
    }
}

int pci_find_class(uint32_t class_mask, PciDev* out)
{
    uint16_t want = (uint16_t)(class_mask >> 8);
    for (uint16_t b = 0; b < 256; b++) {
        for (uint8_t d = 0; d < 32; d++) {
            if (pci_read_config((uint8_t)b, d, 0, 0) == 0xFFFFFFFFu)
                continue;
            for (uint8_t f = 0; f < 8; f++) {
                if (pci_read_config((uint8_t)b, d, f, 0) == 0xFFFFFFFFu)
                    continue;
                PciDev dev;
                pci_read_dev(&dev, (uint8_t)b, d, f);
                uint16_t have =
                    (uint16_t)((uint32_t)dev.class_code << 8 | dev.subclass);
                if (want && have != want)
                    continue;
                if (out)
                    pci_read_dev(out, (uint8_t)b, d, f);
                return 1;
            }
        }
    }
    return 0;
}

} // namespace dev
