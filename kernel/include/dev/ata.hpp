#ifndef KERNEL_DEV_ATA_HPP
#define KERNEL_DEV_ATA_HPP

#include <stdint.h>

namespace dev
{

enum AtaChannel { ATA_PRIMARY = 0, ATA_SECONDARY = 1 };
enum AtaDrive { ATA_MASTER = 0, ATA_SLAVE = 1 };

void ata_init();
int ata_identify(AtaChannel ch, AtaDrive drv, uint16_t* buf_256_words);
int ata_read_sectors(AtaChannel ch, AtaDrive drv, uint32_t lba, uint8_t num,
                     uint16_t* buf);

} // namespace dev

#endif
