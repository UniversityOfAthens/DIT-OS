#include "dev/nvme.hpp"
#include "dev/pci.hpp"
#include "arch/x86_64/io.hpp"

namespace dev
{

int nvme_init(const PciDev* pci)
{
    if (!pci)
        return -1;
    uint32_t bar0 = pci->bar[0];
    if (!(bar0 & 1u))
        return -1;
    (void)bar0;
    return 0;
}

int nvme_read(uint64_t lba, uint32_t num_blocks, void* buf)
{
    (void)lba;
    (void)num_blocks;
    (void)buf;
    return -1;
}

} // namespace dev
