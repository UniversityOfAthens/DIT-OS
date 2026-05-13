// =============================================================================
// DIT-OS Kernel - Early Initialization
// Runs before main kernel logic: BSS clear, multiboot validation, early setup.
// =============================================================================

#include <stdint.h>
#include <stddef.h>

// From entry.asm
extern "C" uint32_t boot_magic;
extern "C" uint32_t boot_info;

// From linker.ld
extern "C" uint8_t __bss_start[];
extern "C" uint8_t __bss_end[];

namespace boot
{

// Multiboot 1 magic (GRUB passes this in EAX when loading Multiboot1 kernel)
constexpr uint32_t MULTIBOOT1_MAGIC = 0x2BADB002;

// Multiboot2 magic (passed in EAX when using Multiboot2)
constexpr uint32_t MULTIBOOT2_MAGIC = 0x36d76289;

// Clear BSS segment (uninitialized data) to zero
static void clear_bss()
{
    for (uint8_t* p = __bss_start; p < __bss_end; ++p) {
        *p = 0;
    }
}

// Validate multiboot and return boot loader type (1 or 2), or 0 if invalid
static uint32_t check_multiboot()
{
    if (boot_magic == MULTIBOOT1_MAGIC) {
        return 1;
    }
    if (boot_magic == MULTIBOOT2_MAGIC) {
        return 2;
    }
    return 0;
}

// Public: call from kernel_main first
// BSS is already cleared by entry.asm (must happen before stack setup)
void early_init()
{
    (void)check_multiboot();
}

// Optional: get multiboot boot info pointer (Multiboot1: physical address in
// EBX)
uint32_t get_boot_info()
{
    return boot_info;
}

// Optional: get multiboot loader type (1, 2, or 0)
uint32_t get_boot_magic()
{
    return boot_magic;
}

} // namespace boot
