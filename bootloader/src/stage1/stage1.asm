; =============================================================================
; DIT-OS Stage 1 Bootloader
; =============================================================================
ORG 0x7C00

COM1_BASE equ 0x3F8
COM_LSR   equ 5
COM_DATA  equ 0
COM_LCR   equ 3
COM_DLL   equ 0
COM_DLH   equ 1

start:
    mov [boot_drive], dl
    
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x5000
    sti
    
    call serial_init
    mov si, msg_startup
    call print_both
    
    ; Reset disk
    mov si, msg_reset
    call print_both
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Read stage2 (9 sectors from LBA 2; CHS sector is 1-based so cl=3 = LBA 2)
    mov si, msg_read
    call print_both
    mov bx, 0x7E00
    mov ah, 0x02
    mov al, 9
    mov ch, 0
    mov cl, 3                 ; CHS sector 3 = LBA 2 (first sector of stage2)
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    mov si, msg_loaded
    call print_both
    mov si, msg_jump2
    call print_both
    
    ; Far jump to stage2
    jmp 0x0000:0x7E00

disk_error:
    mov si, err_disk
    call print_both
.hang:
    hlt
    jmp .hang

serial_init:
    pusha
    mov dx, COM1_BASE + COM_LCR
    mov al, 0x80
    out dx, al
    mov dx, COM1_BASE + COM_DLL
    mov al, 1
    out dx, al
    mov dx, COM1_BASE + COM_DLH
    xor al, al
    out dx, al
    mov dx, COM1_BASE + COM_LCR
    mov al, 0x03
    out dx, al
    popa
    ret

serial_putc:
    pusha
    mov dx, COM1_BASE + COM_LSR
.wait:
    in al, dx
    test al, 0x20
    jz .wait
    mov dx, COM1_BASE + COM_DATA
    mov al, bl
    out dx, al
    popa
    ret

print_both:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov bl, al
    call serial_putc
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
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

boot_drive: db 0
msg_startup: db "[1/3] Stage1...", 13, 10, 0
msg_reset:   db "      Reset disk...", 13, 10, 0
msg_read:    db "      Read stage2 (9 sectors)...", 13, 10, 0
msg_loaded:  db "      OK!", 13, 10, 0
msg_jump2:   db "      Jump to stage2.", 13, 10, 0
err_disk:    db " ERR!", 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
