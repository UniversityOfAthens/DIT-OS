; =============================================================================
; DIT-OS Kernel - Multiboot2 Header (32-bit)
; Specification: https://www.gnu.org/software/grub/manual/multiboot2/
; =============================================================================

section .multiboot2
align 8

multiboot2_header_start:
    dd 0xE85250D6                ; Multiboot2 magic
    dd 0                          ; Architecture: 0 = i386 protected mode
    dd multiboot2_header_end - multiboot2_header_start  ; Header length
    dd -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header_start))  ; Checksum

    ; End tag (type 0, size 8) - required terminator
    dw 0                          ; Type: end
    dw 0                          ; Flags
    dd 8                          ; Size

multiboot2_header_end:
