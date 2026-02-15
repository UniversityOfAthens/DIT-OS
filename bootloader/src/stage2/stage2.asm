; =============================================================================
; DIT-OS Stage 2 - Simple Boot Menu
; =============================================================================
bits 16
ORG 0x7E00

_start:
    mov [boot_drive], dl
    
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti
    
    call clear_screen
    
    mov si, msg_stage2
    call print_string
    
    mov byte [selection], 0
    
main_loop:
    call draw_menu
    
.input:
    call read_key
    
    cmp ah, 0x48    ; Up arrow
    je .up
    cmp ah, 0x50    ; Down arrow
    je .down
    cmp al, 13      ; Enter
    je .select
    
    jmp .input
    
 .up:
    cmp byte [selection], 0
    jne .up_dec
    ; wrap to last option (2)
    mov byte [selection], 2
    jmp main_loop
 .up_dec:
    dec byte [selection]
    jmp main_loop
 
 .down:
    cmp byte [selection], 2
    jne .down_inc
    ; wrap to first option (0)
    mov byte [selection], 0
    jmp main_loop
 .down_inc:
    inc byte [selection]
    jmp main_loop
    
.select:
    call boot_selection
    jmp main_loop

draw_menu:
    pusha
    call clear_screen
    
    mov si, msg_title
    call print_string
    
    mov si, msg_subtitle
    call print_string
    
    mov si, msg_title2
    call print_string
    call print_newline
    
    movzx ax, byte [selection]
    
    cmp ax, 0
    je .show0
    mov si, msg_space
    call print_string
    jmp .p0
.show0:
    mov si, msg_arrow
    call print_string
.p0:
    mov si, msg_opt0
    call print_string
    
    cmp ax, 1
    je .show1
    mov si, msg_space
    call print_string
    jmp .p1
.show1:
    mov si, msg_arrow
    call print_string
.p1:
    mov si, msg_opt1
    call print_string
    
    cmp ax, 2
    je .show2
    mov si, msg_space
    call print_string
    jmp .p2
.show2:
    mov si, msg_arrow
    call print_string
.p2:
    mov si, msg_opt2
    call print_string
    
.done:
    popa
    ret

boot_selection:
    pusha
    
    movzx ax, byte [selection]
    
    cmp ax, 0
    je .boot_normal
    
    cmp ax, 1
    je .boot_recovery
    
    cmp ax, 2
    je .reboot
    
    jmp .done
    
.boot_normal:
    mov si, msg_loading
    call print_string
    mov bx, 0x1000
    mov es, bx
    xor bx, bx
    mov ah, 0x02
    mov al, 64
    mov ch, 0
    mov cl, 6
    mov dh, 1
    mov dl, [boot_drive]
    int 0x13
    jc .error
    
    mov si, msg_booting
    call print_string
    
    popa
    mov dl, [boot_drive]
    mov bx, 0x1000
    push bx
    mov ax, 0
    push ax
    retf
    
.boot_recovery:
    mov si, msg_recovery
    call print_string
    jmp .done
    
.reboot:
    mov si, msg_reboot
    call print_string
    xor ax, ax
    int 0x19
    
.error:
    mov si, msg_error
    call print_string
    
.done:
    popa
    ret

clear_screen:
    pusha
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    popa
    ret

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

print_newline:
    pusha
    mov ah, 0x0E
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    popa
    ret

read_key:
    xor ah, ah
    int 0x16
    ret

boot_drive: db 0
selection: db 0

msg_title: db "================================", 13, 10, 0
msg_subtitle: db "      DIT-OS Bootloader", 13, 10, 0
msg_title2: db "================================", 13, 10, 0

msg_arrow: db ">  ", 0
msg_space: db "   ", 0

msg_opt0: db "Boot DIT-OS", 13, 10, 0
msg_opt1: db "Recovery Mode", 13, 10, 0
msg_opt2: db "Reboot", 13, 10, 0

msg_loading: db "Loading kernel...", 13, 10, 0
msg_booting: db "Booting...", 13, 10, 0
msg_recovery: db "Recovery mode not implemented", 13, 10, 0
msg_reboot: db "Rebooting...", 13, 10, 0
msg_error: db "Error!", 13, 10, 0
msg_stage2: db "Stage2 loaded", 13, 10, 0
