#include "kernel.h"

void kernel_main(void) {
    early_init();
    serial_init();
    gdt_init();
    idt_init();
    pmm_init();
    vmm_init();

    printk("DIT-OS Kernel v0.1\n");
    printk("==================\n");
    printk("Kernel initialized successfully!\n");

    while (1) {
        __asm__ volatile("hlt");
    }
}

void early_init(void) {}
void serial_init(void) {}
void gdt_init(void) {}
void idt_init(void) {}
void pmm_init(void) {}
void vmm_init(void) {}
void printk(const char *fmt, ...) { (void)fmt; }
