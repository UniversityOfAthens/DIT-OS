#ifndef KERNEL_DEV_NVME_HPP
#define KERNEL_DEV_NVME_HPP

#include <stdint.h>

namespace dev
{

struct PciDev;

int nvme_init(const PciDev* pci);
int nvme_read(uint64_t lba, uint32_t num_blocks, void* buf);

} // namespace dev

#endif
