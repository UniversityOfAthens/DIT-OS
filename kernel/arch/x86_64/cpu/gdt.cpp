// =============================================================================
// DIT-OS Kernel - GDT setup (table built in C, includes TSS)
// =============================================================================

#include "arch/x86_64/gdt.hpp"
#include "arch/x86_64/tss.hpp"
#include <stdint.h>

extern "C" void load_gdt(void *descriptor);
extern "C" arch::Tss tss;

namespace arch {

static uint8_t gdt[4 * 8] __attribute__((aligned(8)));
static uint8_t gdt_desc[6] __attribute__((aligned(2)));

static void write_gdt_entry(uint8_t *base, int index, uint32_t base_addr, uint32_t limit, uint8_t access, uint8_t flags) {
    uint8_t *e = base + index * 8;
    uint16_t limit_lo = limit & 0xFFFF;
    uint16_t base_lo = base_addr & 0xFFFF;
    uint8_t base_mid = (base_addr >> 16) & 0xFF;
    uint8_t base_hi = (base_addr >> 24) & 0xFF;
    uint8_t flags_limit = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    e[0] = limit_lo & 0xFF;
    e[1] = (limit_lo >> 8) & 0xFF;
    e[2] = base_lo & 0xFF;
    e[3] = (base_lo >> 8) & 0xFF;
    e[4] = base_mid;
    e[5] = access;
    e[6] = flags_limit;
    e[7] = base_hi;
}

void gdt_load() {
    uint32_t tss_addr = reinterpret_cast<uint32_t>(&tss);
    write_gdt_entry(gdt, 0, 0, 0, 0, 0);                    // null
    write_gdt_entry(gdt, 1, 0, 0xFFFFF, 0x9A, 0xCF);       // code
    write_gdt_entry(gdt, 2, 0, 0xFFFFF, 0x92, 0xCF);       // data
    write_gdt_entry(gdt, 3, tss_addr, 103, 0x89, 0x40);    // TSS (limit 104-1, type 0x89)

    uint16_t limit = sizeof(gdt) - 1;
    uint32_t base = reinterpret_cast<uint32_t>(gdt);
    gdt_desc[0] = limit & 0xFF;
    gdt_desc[1] = (limit >> 8) & 0xFF;
    gdt_desc[2] = base & 0xFF;
    gdt_desc[3] = (base >> 8) & 0xFF;
    gdt_desc[4] = (base >> 16) & 0xFF;
    gdt_desc[5] = (base >> 24) & 0xFF;
    load_gdt(gdt_desc);
}

} // namespace arch
