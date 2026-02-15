#ifndef KERNEL_H
#define KERNEL_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

void kernel_main(void);
void early_init(void);
void serial_init(void);
void gdt_init(void);
void idt_init(void);
void pmm_init(void);
void vmm_init(void);
void printk(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
