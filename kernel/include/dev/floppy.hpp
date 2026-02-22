#ifndef KERNEL_DEV_FLOPPY_HPP
#define KERNEL_DEV_FLOPPY_HPP

#include <stdint.h>

namespace dev {

void floppy_init();
int floppy_read_sector(uint8_t head, uint8_t track, uint8_t sector, uint8_t* buf_512);

}

#endif
