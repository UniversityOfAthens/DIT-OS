; =============================================================================
; DIT-OS Kernel - 8259 PIC (Programmable Interrupt Controller)
; =============================================================================

BITS 32

section .text

global pic_send_eoi

; void pic_send_eoi(uint8_t irq); send 0x20 to slave then master
pic_send_eoi:
    mov eax, [esp + 4]
    mov al, 0x20
    cmp byte [esp + 4], 8
    jb .master_only
    out 0xA0, al      ; slave PIC
.master_only:
    out 0x20, al       ; master PIC
    ret
