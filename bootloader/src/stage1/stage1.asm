; =============================================================================
; DIT-OS Stage 1 Bootloader
; 512-byte boot sector loaded at 0x7C00
; MBR partition table support
; =============================================================================
%include "src/common/constants.inc"
ORG 0x7C00

; Constants
stage2_load_addr equ 0x7E00
stage2_sectors equ 40

; -----------------------------------------------------------------------------
; Boot sector entry point
; -----------------------------------------------------------------------------
start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    mov [boot_drive], dl
    mov [boot_partition], si
    
    call clear_screen
    
    mov si, msg_startup
    call print_string
    
    call check_mbr_partitions
    
    call reset_disk
    
    mov bx, stage2_load_addr
    mov ah, 0x02
    mov al, stage2_sectors
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [boot_drive]
    int 0x13
    
    jc disk_error
    
    mov dl, [boot_drive]
    jmp stage2_load_addr

; -----------------------------------------------------------------------------
; Check MBR Partition Table
; -----------------------------------------------------------------------------
check_mbr_partitions:
    pusha
    mov si, partition_table
    xor cx, cx
    
.check_loop:
    cmp cx, 4
    je .done
    
    mov al, [si + 4]
    test al, al
    jz .next_partition
    
    mov al, [si]
    test al, al
    js .active_found
    
.next_partition:
    add si, 16
    inc cx
    jmp .check_loop
    
.active_found:
    mov [active_partition], si
    
.done:
    popa
    ret

; -----------------------------------------------------------------------------
; Reset disk system
; -----------------------------------------------------------------------------
reset_disk:
    pusha
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13
    popa
    ret

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

; -----------------------------------------------------------------------------
; Print string (SI = pointer)
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
; Disk error handler
; -----------------------------------------------------------------------------
disk_error:
    mov si, err_disk
    call print_string
.hang:
    hlt
    jmp .hang

; -----------------------------------------------------------------------------
; Data section
; -----------------------------------------------------------------------------
boot_drive:      db 0
boot_partition:  dw 0
active_partition: dw 0

msg_startup:     db "DIT-OS Stage1...", 13, 10, 0
err_disk:        db "Disk error!", 13, 10, 0

; -----------------------------------------------------------------------------
; MBR Partition Table (4 entries x 16 bytes = 64 bytes)
; Located at offset 446 (0x1BE)
; -----------------------------------------------------------------------------
times 446-($-$$) db 0

partition_table:
    times 16 db 0
    times 16 db 0
    times 16 db 0
    times 16 db 0

; Boot signature
dw 0xAA55
