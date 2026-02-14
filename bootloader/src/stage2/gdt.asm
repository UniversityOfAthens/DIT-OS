; =============================================================================
; Global Descriptor Table
; =============================================================================
SECTION .data

gdt_start:
; Null descriptor
gdt_null:
    dq 0

; Code segment descriptor
gdt_code:
    dw 0xFFFF       ; Limit
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (mid)
    db 10011010b    ; Access byte
    db 11001111b    ; Flags + Limit (high)
    db 0x00         ; Base (high)

; Data segment descriptor
gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

GLOBAL gdt_descriptor

SECTION .text

GLOBAL load_gdt

load_gdt:
    lgdt [gdt_descriptor]
    ret
