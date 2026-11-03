; =============================================================================
; DIT-OS Kernel - Interrupt Descriptor Table (IDT)
; 256 interrupt gates; each stub pushes vector (and 0 for error_code if none)
; then jumps to common handler which calls C.
; =============================================================================

BITS 32

section .text

; Common handler: vector and error_code are on stack. Call C then iret.
extern interrupt_dispatch

%macro isr_noerr 1
isr_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro isr_err 1
isr_%1:
    push dword %1
    jmp isr_common
%endmacro

; Exceptions 8, 10, 11, 12, 13, 14, 17 push error code; others don't
%assign vec 0
%rep 8
    isr_noerr vec
    %assign vec vec+1
%endrep
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_err 17
%assign vec 18
%rep 238
    isr_noerr vec
    %assign vec vec+1
%endrep

isr_common:
    ; The interrupted context may have DF set (user code can std, and
    ; sigreturn restores it); compiled kernel code assumes DF = 0.
    cld
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, [esp + 48]   ; vector (after pusha=32 + segments=16 = 48 bytes)
    mov ebx, [esp + 52]   ; error_code
    lea ecx, [esp + 16]   ; regs pointer (pusha block starts after 4 segment regs)
    push ecx               ; regs (3rd arg)
    push ebx               ; error_code (2nd arg)
    push eax               ; vector (1st arg)
    call interrupt_dispatch
    add esp, 12
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

; Array of ISR handler addresses for C to build IDT
section .data
global idt_stub_addrs
idt_stub_addrs:
%assign vec 0
%rep 256
    dd isr_ %+ vec
    %assign vec vec+1
%endrep

section .text
; void load_idt(void *descriptor); descriptor is 6 bytes: limit(2), base(4)
global load_idt
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    ret
