#include "dev/ata.hpp"
#include "arch/x86_64/io.hpp"

namespace dev
{

static constexpr uint16_t PRI_BASE = 0x1F0;
static constexpr uint16_t PRI_CTL = 0x3F6;
static constexpr uint16_t SEC_BASE = 0x170;
static constexpr uint16_t SEC_CTL = 0x376;

enum
{
    DATA = 0,
    SEC_CNT = 2,
    LBA_LO = 3,
    LBA_MID = 4,
    LBA_HI = 5,
    DRV = 6,
    CMD = 7
};
enum
{
    CMD_IDENTIFY = 0xEC,
    CMD_READ = 0x20
};
enum
{
    STATUS_BSY = 0x80,
    STATUS_DRQ = 0x08,
    STATUS_ERR = 0x01
};

static uint16_t base(AtaChannel ch)
{
    return ch == ATA_PRIMARY ? PRI_BASE : SEC_BASE;
}
static uint16_t ctl(AtaChannel ch)
{
    return ch == ATA_PRIMARY ? PRI_CTL : SEC_CTL;
}

static void wait_bsy(uint16_t b)
{
    while (arch::inb(b + CMD) & STATUS_BSY)
    {
    }
}
static void wait_drq(uint16_t b)
{
    while (!(arch::inb(b + CMD) & STATUS_DRQ))
    {
    }
}

void ata_init()
{
    arch::outb(ctl(ATA_PRIMARY), 0);
    arch::outb(ctl(ATA_SECONDARY), 0);
}

int ata_identify(AtaChannel ch, AtaDrive drv, uint16_t* buf_256_words)
{
    uint16_t b = base(ch);
    wait_bsy(b);
    arch::outb(b + DRV, 0xA0u | (drv == ATA_SLAVE ? 0x10u : 0u));
    arch::outb(b + SEC_CNT, 0);
    arch::outb(b + LBA_LO, 0);
    arch::outb(b + LBA_MID, 0);
    arch::outb(b + LBA_HI, 0);
    arch::outb(b + CMD, CMD_IDENTIFY);
    wait_bsy(b);
    if (arch::inb(b + LBA_MID) || arch::inb(b + LBA_HI))
        return -1;
    wait_drq(b);
    for (int i = 0; i < 256; i++)
        buf_256_words[i] = arch::inw(b + DATA);
    return 0;
}

int ata_read_sectors(AtaChannel ch, AtaDrive drv, uint32_t lba, uint8_t num,
                     uint16_t* buf)
{
    if (num == 0)
        return -1;
    uint16_t b = base(ch);
    wait_bsy(b);
    arch::outb(b + SEC_CNT, num);
    arch::outb(b + LBA_LO, (uint8_t)(lba));
    arch::outb(b + LBA_MID, (uint8_t)(lba >> 8));
    arch::outb(b + LBA_HI, (uint8_t)(lba >> 16));
    arch::outb(b + DRV,
               0xE0u | (drv == ATA_SLAVE ? 0x10u : 0u) | (uint8_t)(lba >> 24));
    arch::outb(b + CMD, CMD_READ);
    for (uint8_t s = 0; s < num; s++)
    {
        wait_bsy(b);
        if (arch::inb(b + CMD) & STATUS_ERR)
            return -1;
        wait_drq(b);
        for (int i = 0; i < 256; i++)
            *buf++ = arch::inw(b + DATA);
    }
    return 0;
}

} // namespace dev
