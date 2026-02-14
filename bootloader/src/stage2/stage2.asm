; =============================================================================
; DIT-OS Stage 2 Bootloader
; Configuration parsing, menu system, and kernel loading
; =============================================================================
%include "constants.inc"

ORG 0x7E00

SECTION .text

GLOBAL _start

; -----------------------------------------------------------------------------
; Entry point
; -----------------------------------------------------------------------------
_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti
    
    mov [boot_drive], dl
    
    call clear_screen
    
    mov si, msg_stage2
    call print_string
    
    call load_config
    
    mov si, msg_init_menu
    call print_string
    
    call init_menu
    
    call main_loop
    
.hang:
    hlt
    jmp .hang

; -----------------------------------------------------------------------------
; Load configuration file from disk
; -----------------------------------------------------------------------------
load_config:
    pusha
    
    mov si, msg_loadcfg
    call print_string
    
    mov bx, config_buffer
    mov ah, 0x02
    mov al, CONFIG_SECTORS
    mov ch, CONFIG_CYLINDER
    mov cl, CONFIG_SECTOR
    mov dh, CONFIG_HEAD
    mov dl, [boot_drive]
    int 0x13
    
    jc .load_error
    
    call parse_config
    
    cmp byte [entry_count], 0
    je .load_error
    
    popa
    ret
    
.load_error:
    mov si, err_config
    call print_string
    call setup_default_entries
    mov si, msg_after_defaults
    call print_string
    popa
    ret

; -----------------------------------------------------------------------------
; Setup default boot entries when config not found
; -----------------------------------------------------------------------------
setup_default_entries:
    pusha
    cld
    
    mov byte [entry_count], 3
    
    mov si, msg_defaults
    call print_string
    
    mov ax, entries
    shr ax, 8
    call print_hex_byte
    mov ax, entries
    call print_hex_byte
    mov al, 13
    mov ah, 0x0E
    int 0x10
    mov al, 10
    int 0x10
    
    mov di, entries
    
    mov si, def_entry1_name
    mov cx, 32
    rep movsb
    mov si, def_entry1_kernel
    mov cx, 64
    rep movsb
    mov si, def_entry1_cmdline
    mov cx, 128
    rep movsb
    
    mov si, def_entry2_name
    mov cx, 32
    rep movsb
    mov si, def_entry2_kernel
    mov cx, 64
    rep movsb
    mov si, def_entry2_cmdline
    mov cx, 128
    rep movsb
    
    mov si, def_entry3_name
    mov cx, 32
    rep movsb
    mov si, def_entry3_kernel
    mov cx, 64
    rep movsb
    mov si, def_entry3_cmdline
    mov cx, 128
    rep movsb
    
    mov si, msg_copies_done
    call print_string
    
    popa
    ret

print_hex_byte:
    push ax
    push bx
    mov bl, al
    shr al, 4
    call print_hex_nibble
    mov al, bl
    call print_hex_nibble
    pop bx
    pop ax
    ret

print_hex_nibble:
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jle .print
    add al, 7
.print:
    mov ah, 0x0E
    int 0x10
    ret

; -----------------------------------------------------------------------------
; Initialize menu state
; -----------------------------------------------------------------------------
init_menu:
    pusha
    mov byte [menu_selection], 0
    mov byte [timeout_counter], 5
    mov byte [timeout_active], 1
    mov dword [last_tick], 0
    popa
    ret

; -----------------------------------------------------------------------------
; Main menu loop with timeout
; -----------------------------------------------------------------------------
main_loop:
.loop:
    call clear_screen
    call draw_menu
    
    call check_timeout
    
    call wait_input
    
    jmp .loop

; -----------------------------------------------------------------------------
; Check and handle timeout
; -----------------------------------------------------------------------------
check_timeout:
    pusha
    
    cmp byte [timeout_active], 0
    je .done
    
    mov ah, 0x01
    int 0x16
    jnz .key_pressed
    
    mov ah, 0x00
    int 0x1A
    cmp dx, [last_tick]
    je .done
    
    mov [last_tick], dx
    
    dec byte [timeout_counter]
    jns .update_display
    
    mov byte [timeout_active], 0
    call boot_default
    jmp .done
    
.update_display:
    call update_timeout_display
    
.key_pressed:
    mov byte [timeout_active], 0
    
.done:
    popa
    ret

; -----------------------------------------------------------------------------
; Update timeout display
; -----------------------------------------------------------------------------
update_timeout_display:
    pusha
    
    mov dh, 22
    mov dl, 0
    call set_cursor
    
    mov si, msg_timeout
    call print_string
    
    mov al, [timeout_counter]
    add al, '0'
    mov ah, 0x0E
    int 0x10
    
    mov si, msg_seconds
    call print_string
    
    popa
    ret

; -----------------------------------------------------------------------------
; Wait for keyboard input
; -----------------------------------------------------------------------------
wait_input:
    pusha
    
.wait:
    mov ah, 0x01
    int 0x16
    jz .wait
    
    xor ah, ah
    int 0x16
    
    cmp ah, 0x48
    je .up
    cmp ah, 0x50
    je .down
    cmp al, 0x0D
    je .enter
    cmp al, '1'
    jb .wait
    cmp al, '9'
    ja .wait
    
    sub al, '1'
    mov bl, [entry_count]
    cmp al, bl
    jae .wait
    
    mov [menu_selection], al
    jmp .boot_selected
    
.up:
    mov al, [menu_selection]
    test al, al
    jz .wait
    dec al
    mov [menu_selection], al
    popa
    ret
    
.down:
    mov al, [menu_selection]
    mov bl, [entry_count]
    dec bl
    cmp al, bl
    jae .wait
    inc al
    mov [menu_selection], al
    popa
    ret
    
.enter:
    call boot_selected
    popa
    ret
    
.boot_selected:
    call boot_selected
    popa
    ret

; -----------------------------------------------------------------------------
; Boot selected entry
; -----------------------------------------------------------------------------
boot_selected:
    pusha
    
    call clear_screen
    
    mov si, msg_booting
    call print_string
    
    movzx ax, byte [menu_selection]
    mov bx, ENTRY_SIZE
    mul bx
    mov si, entries
    add si, ax
    
    mov di, kernel_path
    add si, 2
    mov cx, 64
    rep movsb
    
    mov si, kernel_path
    call print_string
    call print_newline
    
    call load_kernel
    
    popa
    ret

; -----------------------------------------------------------------------------
; Boot default entry
; -----------------------------------------------------------------------------
boot_default:
    pusha
    mov al, [default_entry]
    mov [menu_selection], al
    call boot_selected
    popa
    ret

; -----------------------------------------------------------------------------
; Load kernel from disk
; -----------------------------------------------------------------------------
load_kernel:
    pusha
    
    mov si, msg_loadkernel
    call print_string
    
    mov bx, kernel_load_seg
    mov es, bx
    xor bx, bx
    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, KERNEL_CYLINDER
    mov cl, KERNEL_SECTOR
    mov dh, KERNEL_HEAD
    mov dl, [boot_drive]
    int 0x13
    
    jc .load_error
    
    mov si, msg_kernel_ok
    call print_string
    
    mov si, msg_jump
    call print_string
    
    popa
    
    mov dl, [boot_drive]
    mov bx, kernel_load_seg
    push bx
    mov ax, kernel_load_off
    push ax
    retf
    
.load_error:
    mov si, err_kernel
    call print_string
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
; Set cursor position (DH = row, DL = column)
; -----------------------------------------------------------------------------
set_cursor:
    pusha
    xor bh, bh
    mov ah, 0x02
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
; Print newline
; -----------------------------------------------------------------------------
print_newline:
    pusha
    mov al, 13
    mov ah, 0x0E
    int 0x10
    mov al, 10
    int 0x10
    popa
    ret

; -----------------------------------------------------------------------------
; Parse configuration file
; -----------------------------------------------------------------------------
parse_config:
    pusha
    
    mov si, config_buffer
    mov di, entries
    mov byte [entry_count], 0
    
.parse_loop:
    call skip_whitespace
    cmp byte [si], 0
    je .done
    cmp byte [si], '#'
    je .skip_line
    cmp byte [si], 'T'
    je .parse_timeout
    cmp byte [si], 'D'
    je .parse_default
    cmp byte [si], 'E'
    je .parse_entry
    
    jmp .next_line
    
.skip_line:
    call skip_to_newline
    jmp .parse_loop
    
.next_line:
    call skip_to_newline
    jmp .parse_loop
    
.parse_timeout:
    add si, 8
    call skip_whitespace
    call parse_number
    mov [timeout_value], al
    jmp .next_line
    
.parse_default:
    add si, 8
    call skip_whitespace
    call parse_number
    mov [default_entry], al
    jmp .next_line
    
.parse_entry:
    mov bl, [entry_count]
    cmp bl, MAX_ENTRIES
    jae .done
    
    mov al, bl
    mov ah, ENTRY_SIZE
    mul ah
    mov di, entries
    add di, ax
    
    add si, 6
    call skip_whitespace
    
    mov al, '"'
    cmp byte [si], al
    jne .parse_entry_done
    inc si
    
    mov cx, 32
    rep movsb
    
    call skip_to_newline
    
.parse_entry_kernel:
    call skip_whitespace
    cmp word [si], 'KE'
    jne .parse_entry_cmdline
    add si, 7
    call skip_whitespace
    
    mov cx, 64
    rep movsb
    
.parse_entry_cmdline:
    call skip_to_newline
    call skip_whitespace
    cmp word [si], 'CM'
    jne .parse_entry_done
    add si, 7
    call skip_whitespace
    
    mov al, '"'
    cmp byte [si], al
    jne .parse_entry_done
    inc si
    
    mov cx, 128
    rep movsb
    
.parse_entry_done:
    inc byte [entry_count]
    jmp .parse_loop
    
.done:
    mov al, [timeout_value]
    mov [timeout_counter], al
    popa
    ret

; -----------------------------------------------------------------------------
; Skip whitespace characters
; -----------------------------------------------------------------------------
skip_whitespace:
    push ax
.loop:
    mov al, [si]
    cmp al, ' '
    je .skip
    cmp al, 9
    je .skip
    cmp al, 13
    je .skip
    cmp al, 10
    je .skip
    jmp .done
.skip:
    inc si
    jmp .loop
.done:
    pop ax
    ret

; -----------------------------------------------------------------------------
; Skip to newline
; -----------------------------------------------------------------------------
skip_to_newline:
    push ax
.loop:
    mov al, [si]
    test al, al
    jz .done
    cmp al, 10
    je .skip_one
    cmp al, 13
    je .skip_one
    inc si
    jmp .loop
.skip_one:
    inc si
.done:
    pop ax
    ret

; -----------------------------------------------------------------------------
; Parse number from string
; -----------------------------------------------------------------------------
parse_number:
    push cx
    push bx
    xor al, al
    xor cx, cx
.loop:
    mov cl, [si]
    cmp cl, '0'
    jb .done
    cmp cl, '9'
    ja .done
    sub cl, '0'
    mov bl, 10
    mul bl
    add al, cl
    inc si
    jmp .loop
.done:
    pop bx
    pop cx
    ret

; -----------------------------------------------------------------------------
; Draw menu
; -----------------------------------------------------------------------------
draw_menu:
    pusha
    
    mov si, msg_draw
    call print_string
    
    mov dh, 1
    mov dl, 0
    call set_cursor
    
    mov si, msg_header
    call print_string
    
    mov dh, 3
    call set_cursor
    
    mov si, msg_title
    call print_string
    
    mov al, [entry_count]
    add al, '0'
    mov ah, 0x0E
    int 0x10
    
    mov dh, 6
    mov dl, 5
    call set_cursor
    
    xor bl, bl
    
.draw_entry:
    mov al, bl
    cmp al, [entry_count]
    jae .draw_footer
    
    cmp bl, [menu_selection]
    jne .draw_normal
    
    mov si, msg_selected
    call print_string
    jmp .draw_name
    
.draw_normal:
    mov si, msg_normal
    call print_string
    
.draw_name:
    mov al, bl
    mov ah, ENTRY_SIZE
    mul ah
    mov si, entries
    add si, ax
    
    mov cx, 32
.draw_name_loop:
    lodsb
    test al, al
    jz .draw_name_done
    mov ah, 0x0E
    int 0x10
    loop .draw_name_loop
    
.draw_name_done:
    call print_newline
    
    inc bl
    jmp .draw_entry
    
.draw_footer:
    mov dh, 20
    mov dl, 0
    call set_cursor
    
    mov si, msg_footer
    call print_string
    
    popa
    ret

; -----------------------------------------------------------------------------
; Data section
; -----------------------------------------------------------------------------
SECTION .data

boot_drive:      db 0
menu_selection:  db 0
entry_count:     db 0
default_entry:   db 0
timeout_value:   db 5
timeout_counter: db 5
timeout_active:  db 1
last_tick:       dd 0

CONFIG_SECTORS   equ 4
CONFIG_CYLINDER  equ 0
CONFIG_SECTOR    equ 20
CONFIG_HEAD      equ 0

KERNEL_SECTORS   equ 64
KERNEL_CYLINDER  equ 0
KERNEL_SECTOR    equ 24
KERNEL_HEAD      equ 0

kernel_load_seg  equ 0x1000
kernel_load_off  equ 0x0000

MAX_ENTRIES      equ 8
ENTRY_SIZE       equ 224

msg_stage2:      db "Stage2 loaded...", 13, 10, 0
msg_init_menu:   db "Init menu...", 13, 10, 0
msg_loadcfg:     db "Loading config...", 13, 10, 0
msg_booting:     db 13, 10, "Booting: ", 0
msg_loadkernel:  db "Loading kernel...", 13, 10, 0
msg_kernel_ok:   db "Kernel loaded!", 13, 10, 0
msg_jump:        db "Jumping to kernel...", 13, 10, 0
msg_timeout:     db "Auto-boot in ", 0
msg_seconds:     db " seconds (press any key)...", 0

msg_header:      db "========================================", 13, 10, 0
msg_title:       db "        DIT-OS Boot Manager", 0
msg_draw:        db "DRAW!", 0
msg_footer:      db "Arrow keys: select | Enter: boot | 1-9: quick select", 13, 10, 0
msg_selected:    db "  > ", 0
msg_normal:      db "    ", 0

err_config:      db "Config load error, using defaults", 13, 10, 0
err_kernel:      db "Kernel load error!", 13, 10, 0
msg_defaults:    db "Defaults loaded.", 13, 10, 0
msg_copies_done: db "Copies done.", 13, 10, 0
msg_after_defaults: db "After defaults...", 13, 10, 0

def_entry1_name:    db "DIT OS", 0
                    times 32-($-def_entry1_name) db 0
def_entry1_kernel:  db "/boot/kernel.bin", 0
                    times 64-($-def_entry1_kernel) db 0
def_entry1_cmdline: db "root=/dev/sda1 quiet", 0
                    times 128-($-def_entry1_cmdline) db 0

def_entry2_name:    db "DIT OS (Debug)", 0
                    times 32-($-def_entry2_name) db 0
def_entry2_kernel:  db "/boot/kernel.bin", 0
                    times 64-($-def_entry2_kernel) db 0
def_entry2_cmdline: db "root=/dev/sda1 debug", 0
                    times 128-($-def_entry2_cmdline) db 0

def_entry3_name:    db "Recovery Mode", 0
                    times 32-($-def_entry3_name) db 0
def_entry3_kernel:  db "/boot/recovery.bin", 0
                    times 64-($-def_entry3_kernel) db 0
def_entry3_cmdline: db "single", 0
                    times 128-($-def_entry3_cmdline) db 0

config_buffer:   times 2048 db 0
entries:         times 1792 db 0
kernel_path:     times 128 db 0
