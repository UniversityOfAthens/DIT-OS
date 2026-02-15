; =============================================================================
; DIT-OS Kernel - Global Descriptor Table (GDT) - 32-bit
; =============================================================================

section .data

gdt_start:

; Null descriptor
gdt_null:
    dq 0

; Code segment descriptor (32-bit)
gdt_code:
    dw 0xFFFF       ; Limit
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (middle)
    db 0x9A         ; Access (present, executable, readable)
    db 0xCF         ; Flags (4KB granularity, 32-bit)
    db 0x00         ; Base (high)

; Data segment descriptor
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92
    db 0xCF
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

section .text

global load_gdt
load_gdt:
    lgdt [gdt_descriptor]
    ret
