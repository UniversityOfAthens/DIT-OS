; =============================================================================
; DIT-OS Kernel - Entry Point
; =============================================================================

[BITS 32]

section .text

global _start
extern __bss_start
extern __bss_end

_start:
    ; Save multiboot regs immediately
    mov [boot_magic], eax
    mov [boot_info], ebx

    ; Clear BSS BEFORE setting up stack (stack is in BSS!)
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor al, al
    cld
    rep stosb

    ; Set up stack (now safe, BSS is cleared)
    mov esp, kernel_stack_top

    ; Jump to C++ kernel
    extern kernel_main
    call kernel_main
    
    ; Should never return
    jmp $

; Data (export for C)
section .data
global boot_magic
global boot_info
boot_magic: dd 0
boot_info: dd 0

; BSS
section .bss
align 16
global kernel_stack
global kernel_stack_top
kernel_stack:
    resb 16384
kernel_stack_top:
