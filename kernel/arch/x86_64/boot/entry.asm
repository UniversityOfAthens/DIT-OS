; =============================================================================
; DIT-OS Kernel - Entry Point
; =============================================================================

[BITS 32]

section .text

global _start
_start:
    ; Multiboot magic in EAX, boot info in EBX
    mov [boot_magic], eax
    mov [boot_info], ebx
    
    ; Set up stack
    mov esp, kernel_stack_top
    
    ; GDT/IDT loaded in arch::cpu_early_init()
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
    times 16384 db 0
kernel_stack_top:
