; =============================================================================
; DIT-OS Kernel - Basic entry point
; Loaded at 0x1000:0x0000 (physical address 0x10000) by bootloader
; =============================================================================

[bits 16]           ; Start in real mode (bootloader keeps us in real mode)
[org 0x0000]        ; Loaded at offset 0

SECTION .text

global _start
_start:
    ; Set up segments
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xF000  ; Stack grows down from 0x1F000

    ; Save boot drive
    mov [boot_drive], dl

    ; Clear screen
    mov ax, 0x0003
    int 0x10

    ; Print welcome message
    mov si, msg_welcome
    call print_string

    mov si, msg_loading
    call print_string

    ; Switch to protected mode
    call enable_a20
    call load_gdt
    call enter_protected_mode

    ; Should never reach here
    jmp $

; -----------------------------------------------------------------------------
; Enable A20 line using fast method
; -----------------------------------------------------------------------------
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; -----------------------------------------------------------------------------
; Load GDT
; -----------------------------------------------------------------------------
load_gdt:
    lgdt [gdt_descriptor]
    ret

; -----------------------------------------------------------------------------
; Enter protected mode
; -----------------------------------------------------------------------------
enter_protected_mode:
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

; -----------------------------------------------------------------------------
; Print string (SI = pointer to string)
; -----------------------------------------------------------------------------
print_string:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; -----------------------------------------------------------------------------
; Protected mode entry point
; -----------------------------------------------------------------------------
[bits 32]
protected_mode:
    ; Set up protected mode segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear screen (VGA text mode)
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0F00
    rep stosw

    ; Print protected mode message
    mov esi, msg_protected
    mov edi, 0xB8000 + 160  ; Second line
    call print_string_pm

    ; Halt
    jmp $

; -----------------------------------------------------------------------------
; Print string in protected mode
; -----------------------------------------------------------------------------
print_string_pm:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0F
    stosw
    jmp .loop
.done:
    popa
    ret

; -----------------------------------------------------------------------------
; Data section
; -----------------------------------------------------------------------------
SECTION .data

boot_drive: db 0

msg_welcome: db "DIT-OS Kernel loaded!", 13, 10, 0
msg_loading: db "Switching to protected mode...", 13, 10, 0
msg_protected: db "DIT-OS running in protected mode!", 0

; -----------------------------------------------------------------------------
; GDT (Global Descriptor Table)
; -----------------------------------------------------------------------------
gdt_start:
    dq 0                    ; Null descriptor

gdt_code:                   ; Code segment descriptor
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10011010b            ; Access byte
    db 11001111b            ; Flags + Limit (bits 16-19)
    db 0x00                 ; Base (bits 24-31)

gdt_data:                   ; Data segment descriptor
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10010010b            ; Access byte
    db 11001111b            ; Flags + Limit (bits 16-19)
    db 0x00                 ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT
    dd gdt_start                ; Base of GDT
