; =============================================================================
; DIT-OS Kernel - Interrupt Descriptor Table (IDT)
; =============================================================================

section .data

idt_start:
    ; 32 exception handlers (each is 8 bytes)
    times 256 dq 0
idt_end:

idt_descriptor:
    dw idt_end - idt_start - 1
    dd idt_start

section .text

global load_idt
load_idt:
    lidt [idt_descriptor]
    ret
