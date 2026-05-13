// DIT-OS Kernel - Floppy controller (0x3F0-0x3F7)

#include "dev/floppy.hpp"
#include "arch/x86_64/io.hpp"
#include <stdint.h>

namespace dev
{

static constexpr uint16_t FDC_BASE = 0x3F0;
enum {
    SRA = 0,
    SRB = 1,
    DOR = 2,
    TDR = 3,
    MSR = 4,
    DSR = 4,
    FIFO = 5,
    DIR = 7
};
enum { CMD_SPECIFY = 3, CMD_SEEK = 15, CMD_READ = 0xE6 };

static void wait_msr(uint8_t mask, uint8_t val)
{
    for (int i = 0; i < 1000; i++) {
        if ((arch::inb(FDC_BASE + MSR) & mask) == val)
            return;
    }
}

void floppy_init()
{
    arch::outb(FDC_BASE + DOR, 0x00);
    wait_msr(0x80, 0x80);
    arch::outb(FDC_BASE + DOR, 0x0C);
    arch::outb(FDC_BASE + FIFO, CMD_SPECIFY);
    arch::outb(FDC_BASE + FIFO, 0xDF);
    arch::outb(FDC_BASE + FIFO, 0x02);
}

int floppy_read_sector(uint8_t head, uint8_t track, uint8_t sector,
                       uint8_t* buf_512)
{
    if (!buf_512)
        return -1;
    wait_msr(0x80, 0x80);
    arch::outb(FDC_BASE + DOR, 0x1C);
    arch::outb(FDC_BASE + FIFO, CMD_SEEK);
    arch::outb(FDC_BASE + FIFO, head << 2);
    arch::outb(FDC_BASE + FIFO, track);
    wait_msr(0x80, 0x80);
    arch::outb(FDC_BASE + FIFO, CMD_READ);
    arch::outb(FDC_BASE + FIFO, head << 2);
    arch::outb(FDC_BASE + FIFO, track);
    arch::outb(FDC_BASE + FIFO, head);
    arch::outb(FDC_BASE + FIFO, sector);
    arch::outb(FDC_BASE + FIFO, 2);
    arch::outb(FDC_BASE + FIFO, 18);
    arch::outb(FDC_BASE + FIFO, 0x1B);
    arch::outb(FDC_BASE + FIFO, 0xFF);
    wait_msr(0x80, 0x80);
    for (int i = 0; i < 256; i++) {
        wait_msr(0x40, 0x40);
        uint16_t w = arch::inw(FDC_BASE + FIFO);
        buf_512[i * 2] = (uint8_t)(w & 0xFF);
        buf_512[i * 2 + 1] = (uint8_t)(w >> 8);
    }
    return 0;
}

} // namespace dev