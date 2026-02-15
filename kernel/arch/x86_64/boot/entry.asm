; =============================================================================
; DIT-OS Kernel - Entry Point
; =============================================================================

[BITS 32]

section .text

global _start
_start:
    ; Multiboot2 magic in EAX, boot info in EBX
    mov [boot_magic], eax
    mov [boot_info], ebx
    
    ; Set up stack
    mov esp, kernel_stack_top
    
    ; Load GDT
    extern load_gdt
    call load_gdt
    
    ; Load IDT
    extern load_idt
    call load_idt
    
    ; Jump to C++ kernel
    extern kernel_main
    call kernel_main
    
    ; Should never return
    jmp $

; Data
section .data
boot_magic: dd 0
boot_info: dd 0

; BSS
section .bss
align 16
kernel_stack:
    times 16384 db 0
kernel_stack_top:
