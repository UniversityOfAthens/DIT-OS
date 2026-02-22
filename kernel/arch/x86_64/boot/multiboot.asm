; =============================================================================
; DIT-OS Kernel - Multiboot 1 Header (32-bit)
; Specification: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
; =============================================================================

section .multiboot
align 4

multiboot_header:
    dd 0x1BADB002                    ; Magic
    dd 0                              ; Flags (no optional fields)
    dd -(0x1BADB002 + 0)              ; Checksum: magic + flags + checksum = 0
