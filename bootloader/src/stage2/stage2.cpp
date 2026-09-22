// =============================================================================
// DIT-OS Stage 2 Bootloader - C++ Entry Point
// =============================================================================
#include <stdint.h>
// External assembly functions
extern "C" {
void enable_a20();
void print_char(char c);
void print_string(const char* s);
char read_key();
void disk_read();
}
// Menu functions
void show_menu();
void boot_kernel();
void show_advanced_menu();
// Simple VGA text output
namespace vga
{
uint16_t* const VIDEO_MEMORY = (uint16_t*)0xB8000;
constexpr int COLS = 80;
constexpr int ROWS = 25;

int cursor_x = 0;
int cursor_y = 0;

void put_char(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }

    VIDEO_MEMORY[cursor_y * COLS + cursor_x] = (0x0F << 8) | c;

    cursor_x++;
    if (cursor_x >= COLS) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= ROWS) {
        cursor_y = 0;
    }
}

void write(const char* str)
{
    while (*str) {
        put_char(*str++);
    }
}

void write_line(const char* str)
{
    write(str);
    put_char('\n');
}

void clear()
{
    for (int i = 0; i < COLS * ROWS; i++) {
        VIDEO_MEMORY[i] = (0x0F << 8) | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}
} // namespace vga
// Menu state
namespace menu
{
int selected = 0;
const int max_options = 2;

void draw()
{
    vga::write("======================================\n");
    vga::write("         DIT-OS Bootloader\n");
    vga::write("======================================\n");
    vga::write("\n");

    for (int i = 0; i <= max_options; i++) {
        if (i == selected) {
            vga::write("    >");
        } else {
            vga::write("     ");
        }

        switch (i) {
        case 0:
            vga::write("Boot DIT-OS\n");
            break;
        case 1:
            vga::write("Advanced options\n");
            break;
        case 2:
            vga::write("Reboot\n");
            break;
        }
    }

    vga::write("\n");
    vga::write("Use arrow keys to select, Enter to confirm\n");
}

void handle_input()
{
    char key = read_key();

    if (key == 0x48) { // Up arrow
        if (selected > 0)
            selected--;
    } else if (key == 0x50) { // Down arrow
        if (selected < max_options)
            selected++;
    } else if (key == 0x0D) { // Enter
        switch (selected) {
        case 0:
            boot_kernel();
            break;
        case 1:
            show_advanced_menu();
            break;
        case 2:
            // Reboot
            __asm__ volatile("int $0x19");
            break;
        }
    }
}
} // namespace menu
// Boot the kernel
void boot_kernel()
{
    vga::clear();
    vga::write_line("Booting DIT-OS...");
    vga::write_line("Loading kernel from disk...");

    // TODO: Load kernel from disk
    // For now, just hang
    vga::write_line("\nKernel not found! Press any key...");
    read_key();
}
// Advanced options menu
void show_advanced_menu()
{
    vga::clear();
    vga::write_line("=== Advanced Options ===");
    vga::write_line("");
    vga::write_line("1. Memory Info");
    vga::write_line("2. Reboot");
    vga::write_line("");
    vga::write_line("Press 1 or 2...");

    char key = read_key();

    if (key == '1') {
        vga::clear();
        vga::write_line("Memory Information");
        vga::write_line("TODO: Display memory map");
        vga::write_line("\nPress any key...");
        read_key();
    }

    show_menu();
}
// Main entry point (called from assembly)
extern "C" void stage2_main()
{
    enable_a20();

    vga::clear();

    while (true) {
        vga::clear();
        menu::draw();
        menu::handle_input();
    }
}
