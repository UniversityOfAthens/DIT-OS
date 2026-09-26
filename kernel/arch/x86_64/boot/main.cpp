// =============================================================================
// DIT-OS Kernel - Main Entry Point
// =============================================================================

#include <stdint.h>

// Kernel version
const char* kernel_version = "DIT-OS 0.0.9-alpha";

// Simple VGA text buffer
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

int cursor_x = 0;
int cursor_y = 0;

enum vga_color
{
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
    while (*str)
    {
        if (*str == '\n')
        {
            cursor_y++;
            cursor_x = 0;
        }
        else
        {
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(
                *str, vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
            cursor_x++;
        }

        if (cursor_x >= VGA_WIDTH)
        {
            cursor_x = 0;
            cursor_y++;
        }

        if (cursor_y >= VGA_HEIGHT)
        {
            for (int y = 0; y < VGA_HEIGHT - 1; y++)
            {
                for (int x = 0; x < VGA_WIDTH; x++)
                {
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

    for (int i = 0; i < 8; i++)
    {
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
void seed_file(const char* name, const void* data, uint32_t size);
}
#include "arch/x86_64/control.hpp"
#include "arch/x86_64/idt.hpp"
#include "arch/x86_64/paging.hpp"
#include "core/panic.hpp"
#include "core/printk.hpp"
#include "errno_defs.hpp"
#include "dev/keyboard.hpp"
#include "dev/mouse.hpp"
#include "dev/rtc.hpp"
#include "dev/serial.hpp"
#include "dev/timer.hpp"
#include "graphics/vga.hpp"
#include "mm/vmm.hpp"
#include "sched/sched.hpp"

extern "C" uint32_t kernel_stack_top[];
/* Second user program, published into the root filesystem for execve(). */
extern "C" const uint8_t exec_image_start[];
extern "C" const uint8_t exec_image_end[];

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
    mm::vmm_init();
    arch::paging_load(mm::vmm_kernel_directory());
    arch::paging_enable();
    syscall::init();
    print_both("[OK] CPU management ready\n\n");

    print_both("[*] Memory Info:\n");
    print_both("    Kernel at: 0x00100000\n");
    print_both("    VGA buffer: 0x000B8000\n\n");

    print_both("[*] Serial/Input drivers...\n");
    // Serial was initialized first thing; re-running serial_init() here
    // would switch the COM1 receive interrupt back off.
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
    arch::pic_unmask(dev::SERIAL_COM1_IRQ);
    print_both("[OK] Serial, keyboard, mouse, timer, RTC ready\n\n");

    print_both("======================================\n");
    print_both("  Kernel initialized successfully!\n");
    print_both("  System is now running...\n");
    print_both("======================================\n");

    dev::serial_puts("\r\n[main] before sched_init\r\n");
    sched::sched_init();
    dev::serial_puts("\r\n[main] before ramfs::init\r\n");
    ramfs::init();
    /* Publish the second user program so execve() has something to load. */
    ramfs::seed_file("greeter", exec_image_start,
                     static_cast<uint32_t>(exec_image_end - exec_image_start));
    dev::serial_puts("\r\n[main] ramfs::init done\r\n");
    dev::serial_puts("\r\n[main] enabling interrupts\r\n");
    __asm__ volatile("sti");
    dev::serial_puts("\r\n[main] before sched_start\r\n");
    sched::sched_start(nullptr);
    dev::serial_puts("\r\n[main] after sched_start (unexpected)\r\n");
    for (;;)
        asm volatile("hlt");
}

static const char* const s_exception_names[arch::EXCEPTION_COUNT] = {
    "divide error",
    "debug",
    "NMI",
    "breakpoint",
    "overflow",
    "bound range exceeded",
    "invalid opcode",
    "device not available",
    "double fault",
    "coprocessor segment overrun",
    "invalid TSS",
    "segment not present",
    "stack-segment fault",
    "general protection fault",
    "page fault",
    "reserved",
    "x87 floating-point error",
    "alignment check",
    "machine check",
    "SIMD floating-point error",
    "virtualization exception",
    "control protection exception",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "hypervisor injection exception",
    "VMM communication exception",
    "security exception",
    "reserved",
};

// Everything except syscalls lands here; PIC EOI is already sent by
// interrupt_dispatch.
extern "C" void interrupt_handler(uint32_t vector, uint32_t error_code,
                                  uint32_t* regs)
{
    if (vector == arch::IRQ_BASE + 0)
    {
        dev::timer_tick();
        return;
    }
    if (vector == arch::IRQ_BASE + dev::SERIAL_COM1_IRQ)
    {
        dev::serial_irq();
        return;
    }
    if (vector >= arch::IRQ_BASE && vector < arch::IRQ_BASE + arch::IRQ_COUNT)
    {
        core::printk("[irq] unhandled IRQ %u\n", vector - arch::IRQ_BASE);
        return;
    }
    if (vector < arch::EXCEPTION_COUNT)
    {
        if ((regs[arch::FRAME_CS] & 3u) != 0)
        {
            /* A user page fault is usually demand paging or a copy-on-write
             * break; both are resolved by filling in the page. */
            sched::Task* task = sched::current_task;
            if (vector == arch::VECTOR_PAGE_FAULT && task &&
                task->page_directory &&
                mm::vmm_handle_fault(task->page_directory, arch::read_cr2(),
                                     error_code) == 0)
                return;
            core::printk("[user] %s at 0x%x (error 0x%x) eip 0x%x esp 0x%x "
                         "eax 0x%x ebx 0x%x ecx 0x%x edx 0x%x esi 0x%x "
                         "edi 0x%x pid %u\n",
                         s_exception_names[vector], arch::read_cr2(),
                         error_code, regs[arch::FRAME_EIP],
                         regs[arch::FRAME_ESP], regs[arch::FRAME_EAX],
                         regs[arch::FRAME_EBX], regs[arch::FRAME_ECX],
                         regs[arch::FRAME_EDX], regs[arch::FRAME_ESI],
                         regs[arch::FRAME_EDI], task ? task->pid : 0);
            sched::task_exit(-EFAULT);
        }
        // printk/panicf write to serial and VGA, so the report is visible
        // with -display none too.
        uint32_t cr2 = vector == arch::VECTOR_PAGE_FAULT ? arch::read_cr2() : 0;
        core::panicf("CPU exception %u (%s)\n"
                     "  error=0x%x eip=0x%x cs=0x%x eflags=0x%x cr2=0x%x\n",
                     vector, s_exception_names[vector], error_code,
                     regs[arch::FRAME_EIP], regs[arch::FRAME_CS],
                     regs[arch::FRAME_EFLAGS], cr2);
    }
    core::printk("[irq] unexpected interrupt vector %u\n", vector);
}
