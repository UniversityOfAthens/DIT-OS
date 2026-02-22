#ifndef KERNEL_DEV_AHCI_HPP
#define KERNEL_DEV_AHCI_HPP

#include <stdint.h>

namespace dev {

struct PciDev;

int ahci_init(const PciDev* pci);
int ahci_port_count();

}

#endif
