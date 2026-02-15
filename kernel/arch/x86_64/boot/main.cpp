// =============================================================================
// DIT-OS Kernel - Main Entry Point
// =============================================================================

#include <stdint.h>

// Kernel version
const char *kernel_version = "DIT-OS 0.1.0";

// Simple VGA text buffer
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
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

uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)(color << 8);
}

void print(const char *str) {
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

void print_hex(uint32_t val) {
    const char *hex = "0123456789ABCDEF";
    char buf[11] = "0x00000000";

    for (int i = 0; i < 8; i++) {
        buf[9 - i] = hex[(val >> (i * 4)) & 0xF];
    }

    print(buf);
}

extern "C" void load_gdt();
extern "C" void load_idt();

extern "C" void kernel_main() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_entry(
                ' ', vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_BLACK));
        }
    }

    cursor_x = 0;
    cursor_y = 0;

    print("======================================\n");
    print("  DIT-OS Kernel\n");
    print("  Version: ");
    print(kernel_version);
    print("\n");
    print("======================================\n\n");

    print("[*] Loading GDT...\n");
    load_gdt();
    print("[OK] GDT loaded\n\n");

    print("[*] Loading IDT...\n");
    load_idt();
    print("[OK] IDT loaded\n\n");

    print("[*] Memory Info:\n");
    print("    Kernel at: 0x00100000\n");
    print("    VGA buffer: 0x000B8000\n\n");

    print("======================================\n");
    print("  Kernel initialized successfully!\n");
    print("  System is now running...\n");
    print("======================================\n");

    while (1) {
        asm volatile("hlt");
    }
}

extern "C" void interrupt_handler(uint32_t vector) {
    print("\n[ERROR] Interrupt! Vector: ");

    char buf[4];
    buf[0] = '0' + (vector / 10);
    buf[1] = '0' + (vector % 10);
    buf[2] = '\0';
    print(buf);
    print("\n");

    while (1) {
        asm volatile("hlt");
    }
}
