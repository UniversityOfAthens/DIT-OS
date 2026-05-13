#include "dev/ahci.hpp"
#include "dev/pci.hpp"
#include "arch/x86_64/io.hpp"

namespace dev
{

static int s_port_count;

int ahci_init(const PciDev* pci)
{
    if (!pci)
        return -1;
    uint32_t bar5 = pci->bar[5];
    if (!(bar5 & 1u))
        return -1;
    uint16_t base = (uint16_t)(bar5 & 0xFFFCu);
    if (base == 0 || base == 0xFFFC)
        return -1;
    uint32_t cap = arch::inl(base + 0x00);
    s_port_count = (int)(cap & 0x1Fu) + 1;
    return 0;
}

int ahci_port_count()
{
    return s_port_count;
}

} // namespace dev
