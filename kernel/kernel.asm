; =============================================================================
; DIT-OS Kernel - Working protected mode entry
; Loaded at 0x1000:0x0000 (physical 0x10000) by bootloader
; =============================================================================

COM1     equ 0x3F8
COM_LSR  equ 5
COM_DATA equ 0
COM_LCR  equ 3
COM_DLL  equ 0
COM_DLH  equ 1

[bits 16]
[org 0x0000]

_start:
    ; Set up segments
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xF000

    ; Init serial (so -nographic shows output)
    call serial_init_16
    mov si, msg_welcome
    call print_serial_16

    ; Clear screen
    mov ax, 0x0003
    int 0x10

    ; Print welcome (VGA/BIOS)
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
    mov si, msg_pm_enter
    call print_serial_16
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

serial_init_16:
    pusha
    mov dx, COM1 + COM_LCR
    mov al, 0x80
    out dx, al
    mov dx, COM1 + COM_DLL
    mov al, 1
    out dx, al
    mov dx, COM1 + COM_DLH
    xor al, al
    out dx, al
    mov dx, COM1 + COM_LCR
    mov al, 0x03
    out dx, al
    popa
    ret

print_serial_16:
    pusha
.ps16_loop:
    lodsb
    test al, al
    jz .ps16_done
    mov bl, al
    mov dx, COM1 + COM_LSR
.ps16_wait:
    in al, dx
    test al, 0x20
    jz .ps16_wait
    mov dx, COM1 + COM_DATA
    mov al, bl
    out dx, al
    jmp .ps16_loop
.ps16_done:
    popa
    ret

[bits 32]
print_serial_32_pm:
    pusha
    mov esi, msg_pm_ok
.ps32_loop:
    lodsb
    test al, al
    jz .ps32_done
    mov bl, al
    mov dx, COM1 + COM_LSR
.ps32_wait:
    in al, dx
    test al, 0x20
    jz .ps32_wait
    mov dx, COM1 + COM_DATA
    mov al, bl
    out dx, al
    jmp .ps32_loop
.ps32_done:
    popa
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

    ; Print to serial (visible with -nographic)
    call print_serial_32_pm
    ; Print success to VGA
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
msg_welcome:  db "Kernel at 0x10000: DIT-OS loaded!", 13, 10, 0
msg_pm_enter: db "  Entering protected mode...", 13, 10, 0

[bits 32]
msg_pm_ok: db 13, 10, "  Protected mode active. PM!", 13, 10, 0

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
