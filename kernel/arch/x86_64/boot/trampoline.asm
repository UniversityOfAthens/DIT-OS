; =============================================================================
; DIT-OS Kernel - SMP Application Processor (AP) Trampoline
; This code is built as a flat binary with ORG 0x1000 and must be copied
; to physical address 0x1000 before sending INIT-SIPI-SIPI to APs.
; =============================================================================

ORG 0x1000
BITS 16

ap_trampoline_start:
    ; Entry: AP starts here in real mode (usually after INIT-SIPI-SIPI)
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Enable A20 (keyboard controller method - safe for AP)
    call .a20_wait
    mov al, 0xAD
    out 0x64, al
    call .a20_wait
    mov al, 0xD0
    out 0x64, al
    call .a20_wait2
    in al, 0x60
    push eax
    call .a20_wait
    mov al, 0xD1
    out 0x64, al
    call .a20_wait
    pop eax
    or al, 2
    out 0x60, al
    call .a20_wait
    mov al, 0xAE
    out 0x64, al
    call .a20_wait

    ; Load minimal GDT for protected mode
    lgdt [trampoline_gdtr]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:.pmode
.a20_wait:
    in al, 0x64
    test al, 2
    jnz .a20_wait
    ret
.a20_wait2:
    in al, 0x64
    test al, 1
    jz .a20_wait2
    ret

BITS 32
.pmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9000

    ; AP entry point: BSP will patch this with a jump to ap_entry() in C,
    ; or we could use a fixed address. For now just halt until SMP init exists.
    cli
.hang:
    hlt
    jmp .hang

BITS 16
align 8
trampoline_gdt:
    dq 0
    ; Code segment
    dw 0xFFFF, 0
    db 0, 0x9A, 0xCF, 0
    ; Data segment
    dw 0xFFFF, 0
    db 0, 0x92, 0xCF, 0
trampoline_gdt_end:

trampoline_gdtr:
    dw trampoline_gdt_end - trampoline_gdt - 1
    dd trampoline_gdt

ap_trampoline_end:
