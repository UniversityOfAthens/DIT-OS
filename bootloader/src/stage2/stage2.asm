; =============================================================================
; DIT-OS Stage 2 - Simple Boot Menu
; =============================================================================
bits 16
ORG 0x7E00

COM1_BASE equ 0x3F8
COM_LSR   equ 5
COM_DATA  equ 0
COM_LCR   equ 3
COM_DLL   equ 0
COM_DLH   equ 1

_start:
    mov [boot_drive], dl
    
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti
    
    call serial_init
    call clear_screen
    
    mov si, msg_stage2
    call print_both
    
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
    cmp al, '1'
    je .set_0
    cmp al, '2'
    je .set_1
    cmp al, '3'
    je .set_2

    jmp .input
 .set_0:
    mov byte [selection], 0
    jmp main_loop
 .set_1:
    mov byte [selection], 1
    jmp main_loop
 .set_2:
    mov byte [selection], 2
    jmp main_loop
    
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
    call print_both
    mov si, msg_subtitle
    call print_both
    mov si, msg_title2
    call print_both
    mov si, msg_crlf
    call print_both
    
    movzx ax, byte [selection]
    
    cmp ax, 0
    je .show0
    mov si, msg_space
    call print_both
    jmp .p0
.show0:
    mov si, msg_arrow
    call print_both
.p0:
    mov si, msg_opt0
    call print_both
    
    cmp ax, 1
    je .show1
    mov si, msg_space
    call print_both
    jmp .p1
.show1:
    mov si, msg_arrow
    call print_both
.p1:
    mov si, msg_opt1
    call print_both
    
    cmp ax, 2
    je .show2
    mov si, msg_space
    call print_both
    jmp .p2
.show2:
    mov si, msg_arrow
    call print_both
.p2:
    mov si, msg_opt2
    call print_both
    mov si, msg_crlf
    call print_both
    mov si, msg_hint
    call print_both

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
    call print_both
    ; Load loader (2 sectors from LBA 11) to 0x1000:0 = 0x10000
    mov word [dap_count], 2
    mov word [dap_offset], 0x0000
    mov word [dap_segment], 0x1000
    mov dword [dap_lba], 11
    call read_lba
    jc .error
    ; Load kernel ELF (110 sectors from LBA 37) to 0x2000:0 = 0x20000
    mov word [dap_count], 110
    mov word [dap_offset], 0x0000
    mov word [dap_segment], 0x2000
    mov dword [dap_lba], 37
    call read_lba
    jc .error
    mov si, msg_read_ok
    call print_both
    mov si, msg_booting
    call print_both
    mov si, msg_jump_kernel
    call print_both
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

; VT100: clear screen and move cursor home (so serial shows one menu per redraw)
serial_clear:
    pusha
    mov si, msg_vt100_clear
    call print_serial_only
    popa
    ret

clear_screen:
    pusha
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    call serial_clear
    popa
    ret

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

; Print string to serial only (for VT100 sequences - no int 0x10)
print_serial_only:
    pusha
.sloop:
    lodsb
    or al, al
    jz .sdone
    mov bl, al
    call serial_putc
    jmp .sloop
.sdone:
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

print_newline:
    pusha
    mov ah, 0x0E
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    popa
    ret

; Check if serial has data (for -serial mon:stdio keyboard)
; Returns ZF=1 if no data, ZF=0 if data available
serial_has_key:
    push dx
    mov dx, COM1_BASE + COM_LSR
    in al, dx
    test al, 1
    pop dx
    ret

; Read key: poll serial and BIOS so we never block on int 0x16 when key is from serial.
; Returns: ah = scan code (0x48 up, 0x50 down), al = ASCII (13 for Enter, '1'/'2'/'3')
; With -serial mon:stdio, arrow keys come as VT100 ESC [ A / ESC [ B; we parse and return ah=0x48/0x50.
read_key:
    jmp .poll
; Wait for next serial byte (timeout so we don't hang). Returns ZF=0 if byte in AL.
.wait_serial_byte:
    push cx
    mov cx, 0
.wait_serial_loop:
    call serial_has_key
    jnz .wait_serial_got
    dec cx
    jnz .wait_serial_loop
    pop cx
    xor al, al
    test al, 1
    ret
.wait_serial_got:
    mov dx, COM1_BASE + COM_DATA
    in al, dx
    pop cx
    test al, al
    ret

.poll:
    call serial_has_key
    jnz .read_serial
    mov ah, 1
    int 0x16
    jnz .use_bios
    jmp .poll
.read_serial:
    push dx
    mov dx, COM1_BASE + COM_DATA
    in al, dx
    pop dx
    cmp al, 0x1B
    je .escape
    cmp al, 0x0D
    je .enter
    cmp al, 0x0A
    je .enter
    ; Normal ASCII (e.g. '1','2','3') from serial: return ah=0, al=byte
    mov ah, 0
    ret
.enter:
    mov ah, 0
    mov al, 13
    ret
.escape:
    call .wait_serial_byte
    jz .poll
    cmp al, '['                  ; ESC [
    je .escape_bracket
    cmp al, 'O'                  ; ESC O (alternative)
    je .escape_O
    jmp .poll
.escape_bracket:
    call .wait_serial_byte
    jz .poll
    cmp al, 'A'
    je .key_up
    cmp al, 'B'
    je .key_down
    jmp .poll
.escape_O:
    call .wait_serial_byte
    jz .poll
    cmp al, 'A'
    je .key_up
    cmp al, 'B'
    je .key_down
    jmp .poll
.key_up:
    mov ah, 0x48
    xor al, al
    ret
.key_down:
    mov ah, 0x50
    xor al, al
    ret
.use_bios:
    xor ah, ah
    int 0x16
    ret

; =============================================================================
; read_lba - Read sectors from LBA using CHS, one sector at a time
; Uses dap_* variables for parameters. Sets CF on error.
; =============================================================================
read_lba:
    pusha
    mov cx, [dap_count]
.rl_loop:
    test cx, cx
    jz .rl_done
    push cx
    ; LBA -> CHS  (18 sectors/track, 2 heads/cylinder)
    mov eax, [dap_lba]
    xor edx, edx
    mov ebx, 18
    div ebx                   ; eax = track, edx = sector-in-track
    inc dx                    ; sectors are 1-based
    mov cl, dl                ; CL = sector
    xor edx, edx
    mov ebx, 2
    div ebx                   ; eax = cylinder, edx = head
    mov ch, al                ; CH = cylinder
    mov dh, dl                ; DH = head
    ; Read 1 sector
    mov ax, [dap_segment]
    mov es, ax
    mov bx, [dap_offset]
    mov ah, 0x02
    mov al, 1
    mov dl, [boot_drive]
    int 0x13
    jc .rl_err
    ; Advance LBA and buffer offset
    inc dword [dap_lba]
    add word [dap_offset], 512
    ; If offset wrapped past 0xFFFF, advance segment
    jnc .rl_no_wrap
    add word [dap_segment], 0x1000
.rl_no_wrap:
    pop cx
    dec cx
    jmp .rl_loop
.rl_done:
    popa
    clc
    ret
.rl_err:
    pop cx
    popa
    stc
    ret

; read_lba parameter block
dap_count:   dw 0
dap_offset:  dw 0
dap_segment: dw 0
dap_lba:     dd 0

boot_drive: db 0
selection: db 0

msg_title: db "================================", 13, 10, 0
msg_subtitle: db "      DIT-OS Bootloader", 13, 10, 0
msg_title2: db "================================", 13, 10, 0

msg_crlf:  db 13, 10, 0
msg_arrow: db ">  ", 0
msg_space: db "   ", 0

msg_opt0: db "Boot DIT-OS", 13, 10, 0
msg_opt1: db "Recovery Mode", 13, 10, 0
msg_opt2: db "Reboot", 13, 10, 0
msg_hint: db "Up/Down or 1/2/3, Enter to select", 13, 10, 0
msg_vt100_clear: db 27, "[2J", 27, "[H", 0

msg_loading:     db "[3/3] Loading loader+kernel...", 13, 10, 0
msg_read_ok:     db "      Read OK.", 13, 10, 0
msg_booting:     db "      Booting...", 13, 10, 0
msg_jump_kernel: db "      Jump to 0x10000.", 13, 10, 0
msg_recovery: db "Recovery mode not implemented", 13, 10, 0
msg_reboot: db "Rebooting...", 13, 10, 0
msg_error: db "Error!", 13, 10, 0
msg_stage2: db "[2/3] Stage2 loaded.", 13, 10, 0
