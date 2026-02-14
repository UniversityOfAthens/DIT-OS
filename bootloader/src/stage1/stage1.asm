; =============================================================================
; DIT-OS Stage 1 Bootloader
; 512-byte boot sector loaded at 0x7C00
; =============================================================================
%include "src/common/constants.inc"
ORG 0x7C00

; Constants - load stage2 at 0x7E00 (right after boot sector)
stage2_load_addr equ 0x7E00
stage2_sectors equ 1

; -----------------------------------------------------------------------------
; Boot sector entry point
; -----------------------------------------------------------------------------
start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Clear screen completely
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    mov [boot_drive], dl
    
    call show_boot_menu
    
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    
    mov bx, stage2_load_addr
    mov ah, 0x02
    mov al, stage2_sectors
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    
    jc disk_error
    
    jmp stage2_load_addr

; -----------------------------------------------------------------------------
; Boot Menu
; -----------------------------------------------------------------------------
show_boot_menu:
    pusha
    mov ah, 0x07
    mov al, 0
    xor cx, cx
    mov dh, 24
    mov dl, 79
    mov bh, 0x07
    int 0x10
    
    xor bh, bh
    xor dx, dx
    mov ah, 0x02
    int 0x10
    
    mov si, menu_title
    call print_string
    mov si, menu_opt1
    call print_string
    mov si, menu_opt2
    call print_string
    mov si, menu_opt3
    call print_string
    
    mov byte [cnt], 3
    
.wait:
    mov ah, 01h
    int 16h
    jnz .key
    
    dec byte [cnt]
    jz .go
    
    mov ah, 86h
    mov cx, 0
    mov dx, 18200
    int 15h
    
    jmp .wait
    
.key:
    xor ah, ah
    int 16h
    cmp al, '3'
    je .reboot
    cmp al, '2'
    je .safe
    
.go:
    popa
    ret

.safe:
    popa
    ret

.reboot:
    int 0x19

; -----------------------------------------------------------------------------
; Print string
; -----------------------------------------------------------------------------
print_string:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp print_string
.done:
    ret

; -----------------------------------------------------------------------------
; Disk error
; -----------------------------------------------------------------------------
disk_error:
    mov si, err_disk
    call print_string
hang:
    jmp hang

; -----------------------------------------------------------------------------
; Data
; -----------------------------------------------------------------------------
boot_drive: db 0
cnt: db 0

menu_title: db 13, 10, "=== DIT-OS Bootloader ===", 13, 10, 0
menu_opt1:  db "1. Boot DIT-OS", 13, 10, 0
menu_opt2:  db "2. Safe Mode", 13, 10, 0
menu_opt3:  db "3. Reboot", 13, 10, 0

err_disk: db "Disk error!", 0

times 446-($-$$) db 0
times 64 db 0
dw 0xAA55
