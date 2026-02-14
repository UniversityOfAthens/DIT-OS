; =============================================================================
; DIT-OS Stage 2 Bootloader
; =============================================================================
%include "constants.inc"

ORG 0x7E00

SECTION .text

GLOBAL _start

_start:
    ; Clear screen completely
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    
    ; Print welcome
    mov si, msg_welcome
    call print_string
    
    ; Menu loop
    mov byte [sel], 0
    
menu_loop:
    call clear_screen
    call draw_menu
    
input_wait:
    xor ah, ah
    int 0x16
    
    cmp al, '1'
    je boot_kernel
    cmp al, '2'
    je advanced
    cmp al, '3'
    je reboot
    
    cmp ah, 48h      ; Up arrow
    je move_up
    cmp ah, 50h      ; Down arrow
    je move_down
    
    jmp input_wait

move_up:
    dec byte [sel]
    cmp byte [sel], 0
    jge menu_loop
    mov byte [sel], 2
    jmp menu_loop

move_down:
    inc byte [sel]
    cmp byte [sel], 2
    jle menu_loop
    mov byte [sel], 0
    jmp menu_loop

boot_kernel:
    mov si, msg_booting
    call print_string
    jmp hang

advanced:
    mov si, msg_advanced
    call print_string
    xor ah, ah
    int 0x16
    jmp menu_loop

reboot:
    int 0x19

hang:
    jmp hang

; -----------------------------------------------------------------------------
; Clear screen
; -----------------------------------------------------------------------------
clear_screen:
    pusha
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    popa
    ret

clear_menu_area:
    ret

; -----------------------------------------------------------------------------
; Draw Menu
; -----------------------------------------------------------------------------
draw_menu:
    pusha
    ; Cursor to row 2 for title
    mov dh, 2
    mov dl, 0
    mov ah, 0x02
    xor bh, bh
    int 0x10
    
    mov si, msg_title
    call print_string
    
    ; Cursor to row 4 for options
    mov dh, 4
    mov dl, 0
    mov ah, 0x02
    xor bh, bh
    int 0x10
    
    ; Option 1
    cmp byte [sel], 0
    jne .opt1_nosel
    mov si, msg_sel
    call print_string
    jmp .opt1_done
.opt1_nosel:
    mov si, msg_spc
    call print_string
.opt1_done:
    mov si, msg_opt1
    call print_string
    
    ; Option 2
    cmp byte [sel], 1
    jne .opt2_nosel
    mov si, msg_sel
    call print_string
    jmp .opt2_done
.opt2_nosel:
    mov si, msg_spc
    call print_string
.opt2_done:
    mov si, msg_opt2
    call print_string
    
    ; Option 3
    cmp byte [sel], 2
    jne .opt3_nosel
    mov si, msg_sel
    call print_string
    jmp .opt3_done
.opt3_nosel:
    mov si, msg_spc
    call print_string
.opt3_done:
    mov si, msg_opt3
    call print_string
    
    popa
    ret

; -----------------------------------------------------------------------------
; Print string
; -----------------------------------------------------------------------------
print_string:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; -----------------------------------------------------------------------------
; Data
; -----------------------------------------------------------------------------
sel: db 0

msg_title:  db 13, 10, "=== Stage 2: DIT-OS Menu ===", 13, 10, 13, 10, 0
msg_sel:    db "> ", 0
msg_spc:    db "  ", 0
msg_opt1:   db "1. Boot DIT-OS", 13, 10, 0
msg_opt2:   db "2. Advanced Options", 13, 10, 0
msg_opt3:   db "3. Reboot", 13, 10, 0

msg_welcome: db "Stage 2 loaded OK!", 13, 10, 0
msg_booting: db 13, 10, "Booting kernel...", 13, 10, 0
msg_advanced: db 13, 10, "Advanced mode - press key...", 13, 10, 0
