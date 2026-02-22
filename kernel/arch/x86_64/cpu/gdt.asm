; =============================================================================
; DIT-OS Kernel - GDT load (table built in C)
; =============================================================================

BITS 32

section .text

; void load_gdt(void *descriptor); descriptor is 6 bytes: limit(2), base(4)
global load_gdt
load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]
    ret
