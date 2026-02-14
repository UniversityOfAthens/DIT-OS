; =============================================================================
; Common NASM Macros
; =============================================================================
; Define a far jump (16-bit code segment)
%macro far_jump 2
    db 0xEA
    dw %1
    dw %2
%endmacro
; Define a far call
%macro far_call 2
    db 0x9A
    dw %1
    dw %2
%endmacro
; Define GDT descriptor
%macro gdt_descriptor 3
    dw %2 & 0xFFFF         ; Limit (low 16 bits)
    dw %1 & 0xFFFF         ; Base (low 16 bits)
    db (%1 >> 16) & 0xFF   ; Base (middle 8 bits)
    db %3                  ; Access byte
    db ((%2 >> 16) & 0x0F) | ((%3 & 0xF0) & 0xF0) ; Limit (high 4 bits) | Flags
    db (%1 >> 24) & 0xFF   ; Base (high 8 bits)
%endmacro
; Print character in AL
%macro print_char 0
    pusha
    mov ah, 0x0E
    int 0x10
    popa
%endmacro
; Print string (SI = pointer)
%macro print_string 0
    pusha
    %%loop:
        lodsb
        or al, al
        jz %%done
        mov ah, 0x0E
        int 0x10
        jmp %%loop
    %%done:
    popa
%endmacro
; Read key press
%macro read_key 0
    xor ah, ah
    int 0x16
%endmacro
; Enable interrupts
%macro enable_interrupts 0
    sti
%endmacro
; Disable interrupts
%macro disable_interrupts 0
    cli
%endmacro