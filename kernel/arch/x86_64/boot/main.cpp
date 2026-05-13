// =============================================================================
// DIT-OS Kernel - Main Entry Point
// =============================================================================

#include <stdint.h>

// Kernel version
const char* kernel_version = "DIT-OS 0.1.0";

// Simple VGA text buffer
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

int cursor_x = 0;
int cursor_y = 0;

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
};

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | (bg << 4);
}

uint16_t vga_entry(unsigned char c, uint8_t color)
{
    return (uint16_t)c | (uint16_t)(color << 8);
}

void print(const char* str)
{
    while (*str) {
        if (*str == '\n') {
            cursor_y++;
            cursor_x = 0;
        } else {
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(
                *str, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            cursor_x++;
        }

        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }

        if (cursor_y >= VGA_HEIGHT) {
            for (int y = 0; y < VGA_HEIGHT - 1; y++) {
                for (int x = 0; x < VGA_WIDTH; x++) {
                    vga_buffer[y * VGA_WIDTH + x] =
                        vga_buffer[(y + 1) * VGA_WIDTH + x];
                }
            }
            cursor_y = VGA_HEIGHT - 1;
        }

        str++;
    }
}

void print_hex(uint32_t val)
{
    const char* hex = "0123456789ABCDEF";
    char buf[11] = "0x00000000";

    for (int i = 0; i < 8; i++) {
        buf[9 - i] = hex[(val >> (i * 4)) & 0xF];
    }

    print(buf);
}

namespace boot
{
void early_init();
}
namespace arch
{
void cpu_early_init();
void tss_set_kernel_stack(uint32_t esp0);
void pic_unmask(uint8_t);
} // namespace arch
namespace syscall
{
void init();
}
namespace ramfs
{
void init();
}
#include "arch/x86_64/idt.hpp"
#include "dev/keyboard.hpp"
#include "dev/mouse.hpp"
#include "dev/rtc.hpp"
#include "dev/serial.hpp"
#include "dev/timer.hpp"
#include "graphics/vga.hpp"
#include "sched/sched.hpp"

extern "C" uint32_t kernel_stack_top[];

/* Print to both VGA and serial (so -display none -serial mon:stdio shows full
 * boot) */
static void print_both(const char* str)
{
    print(str);
    dev::serial_puts(str);
}

extern "C" void kernel_main()
{
    boot::early_init();

    /* Early serial so something appears with QEMU -display none -serial
     * mon:stdio */
    dev::serial_init();
    dev::serial_puts("\r\nDIT-OS Kernel starting...\r\n");

    gfx::vga_clear(gfx::vga_attr(gfx::VGA_BLACK, gfx::VGA_BLACK));

    cursor_x = 0;
    cursor_y = 0;

    print_both("======================================\n");
    print_both("  DIT-OS Kernel\n");
    print("  Version: ");
    print(kernel_version);
    print_both("\n");
    print_both("======================================\n\n");

    print_both("[*] CPU init (GDT, IDT, TSS, PIC)...\n");
    arch::cpu_early_init();
    arch::tss_set_kernel_stack((uint32_t)kernel_stack_top);
    syscall::init();
    print_both("[OK] CPU management ready\n\n");

    print_both("[*] Memory Info:\n");
    print_both("    Kernel at: 0x00100000\n");
    print_both("    VGA buffer: 0x000B8000\n\n");

    print_both("[*] Serial/Input drivers...\n");
    dev::serial_init();
    dev::serial_puts("  Serial (COM1) OK\n");
    print("  Serial (COM1) OK\n");
    dev::keyboard_init();
    dev::mouse_init();
    dev::timer_init();
    dev::rtc_init();
    dev::RtcTime rtc;
    dev::rtc_read(&rtc);
    print_both("  RTC: 20");
    char db[16];
    db[0] = '0' + (rtc.year % 100 / 10);
    db[1] = '0' + (rtc.year % 10);
    db[2] = '-';
    db[3] = '0' + (rtc.month / 10);
    db[4] = '0' + (rtc.month % 10);
    db[5] = '-';
    db[6] = '0' + (rtc.day / 10);
    db[7] = '0' + (rtc.day % 10);
    db[8] = ' ';
    db[9] = '0' + (rtc.hour / 10);
    db[10] = '0' + (rtc.hour % 10);
    db[11] = ':';
    db[12] = '0' + (rtc.minute / 10);
    db[13] = '0' + (rtc.minute % 10);
    db[14] = '\0';
    print_both(db);
    print_both("\n");
    arch::pic_unmask(0);
    print_both("[OK] Serial, keyboard, mouse, timer, RTC ready\n\n");

    print_both("======================================\n");
    print_both("  Kernel initialized successfully!\n");
    print_both("  System is now running...\n");
    print_both("======================================\n");

    dev::serial_puts("\r\n[main] before sched_init\r\n");
    sched::sched_init();
    dev::serial_puts("\r\n[main] before ramfs::init\r\n");
    ramfs::init(); /* mount minimal root filesystem at '/' (needs heap from
                      sched_init) */
    dev::serial_puts("\r\n[main] ramfs::init done\r\n");
    dev::serial_puts(
        "\r\n[main] before sched_start (sti done inside sched_start)\r\n");
    sched::sched_start(
        nullptr); /* does not return; enables interrupts and runs init task */
    dev::serial_puts("\r\n[main] after sched_start (unexpected)\r\n");
    for (;;)
        asm volatile("hlt");
}

extern "C" void interrupt_handler(uint32_t vector)
{
    if (vector == arch::IRQ_BASE + 0) {
        dev::timer_tick();
        sched::schedule();
        return;
    }
    print("\n[ERROR] Interrupt! Vector: ");
    char buf[4];
    buf[0] = '0' + (vector / 10);
    buf[1] = '0' + (vector % 10);
    buf[2] = '\0';
    print(buf);
    print("\n");
    while (1)
        asm volatile("hlt");
}
