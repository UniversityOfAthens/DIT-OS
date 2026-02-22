; =============================================================================
; DIT-OS Kernel - Task State Segment (TSS)
; =============================================================================

BITS 32

section .bss
align 4
global tss
tss:
    resb 104

section .text
global tss_load
tss_load:
    mov ax, 0x18
    ltr ax
    ret
