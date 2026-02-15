; =============================================================================
; DIT-OS Kernel - Working protected mode entry
; Loaded at 0x1000:0x0000 (physical 0x10000) by bootloader
; =============================================================================

[bits 16]
[org 0x0000]

_start:
    ; Set up segments
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xF000

    ; Clear screen
    mov ax, 0x0003
    int 0x10

    ; Print welcome
    mov si, msg_welcome
    call print_string
    
    ; Debug: print 'S' (setup)
    mov ax, 0x0E53
    int 0x10

    ; Set up GDT with correct physical address
    xor eax, eax
    mov ax, ds
    shl eax, 4              ; EAX = 0x10000
    add eax, gdt_start      ; EAX = physical address of GDT
    mov [gdt_descriptor + 2], eax
    mov word [gdt_descriptor], gdt_end - gdt_start - 1

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Debug: print 'E' (entering)
    mov ax, 0x0E45
    int 0x10
    
    ; Enter protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Debug: print 'J' (jumping)
    mov ax, 0x0E4A
    int 0x10

    ; Far jump to protected mode
    jmp 0x08:pm_entry

print_string:
    push ax
    push si
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    pop si
    pop ax
    ret

[bits 32]
pm_entry:
    ; Set up data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Print success message directly
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    mov byte [0xB8002], 'M'
    mov byte [0xB8003], 0x0F
    mov byte [0xB8004], '!'
    mov byte [0xB8005], 0x0F

    ; Hang
    cli
.hang:
    hlt
    jmp .hang

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

[bits 16]
msg_welcome: db "DIT-OS Kernel loaded!", 13, 10, 0

[bits 32]
msg_pm_ok: db "Protected mode active!", 0

[bits 16]
align 4
gdt_descriptor:
    dw 0
    dd 0

align 8
gdt_start:
    dq 0                    ; Null descriptor

gdt_code:                   ; Code segment - base 0x10000, limit 4GB
    dw 0xFFFF               ; Limit 0-15
    dw 0x0000               ; Base 0-15
    db 0x01                 ; Base 16-23 (0x10000 >> 16 = 0x01)
    db 10011010b            ; Access: present, ring 0, code, execute/read
    db 11001111b            ; Flags: 4KB granularity, 32-bit, limit 16-19
    db 0x00                 ; Base 24-31

gdt_data:                   ; Data segment - base 0x10000, limit 4GB
    dw 0xFFFF
    dw 0x0000
    db 0x01
    db 10010010b            ; Access: present, ring 0, data, read/write
    db 11001111b
    db 0x00

gdt_end:
