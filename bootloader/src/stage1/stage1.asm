; =============================================================================
; DIT-OS Stage 1 Bootloader
; =============================================================================
ORG 0x7C00

start:
    mov [boot_drive], dl
    
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x5000
    sti
    
    mov si, msg_startup
    call print_string
    
    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    
    ; Read stage2
    mov bx, 0x7E00
    mov ah, 0x02
    mov al, 9
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    mov si, msg_loaded
    call print_string
    
    ; Far jump to stage2
    jmp 0x0000:0x7E00

disk_error:
    mov si, err_disk
    call print_string
.hang:
    hlt
    jmp .hang

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
msg_startup: db "Stage1...", 0
msg_loaded: db " OK!", 0
err_disk: db " ERR!", 0

times 510-($-$$) db 0
dw 0xAA55
