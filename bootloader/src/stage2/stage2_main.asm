; =============================================================================
; DIT-OS Stage 2 - Main Entry Point (ELF Build)
; =============================================================================
bits 16
%include "constants.inc"

; External functions from stage2_core
extern boot_drive
extern check_vesa
extern clear_screen
extern print_string
extern print_newline
extern read_key
extern force_text
extern force_graphics
extern graphics_available
extern current_mode
extern set_graphics_mode
extern init_menu
extern parse_config
extern entry_count
extern menu_selection
extern config_buffer
extern draw_text_menu

SECTION .text

GLOBAL _start
_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000
    sti

s2_hang:
    hlt
    jmp s2_hang

    mov si, msg_stage2
    call print_string

    call load_config

    ; Setup display mode
    cmp byte [force_text], 1
    je .text_mode
    cmp byte [force_graphics], 1
    je .graphics_mode
    cmp byte [graphics_available], 1
    je .graphics_mode

.text_mode:
    mov byte [current_mode], 0
    jmp .menu_init

.graphics_mode:
    mov byte [current_mode], 1
    call set_graphics_mode

.menu_init:
    mov si, msg_init_menu
    call print_string

    call init_menu

    call main_loop

.hang:
    hlt
    jmp .hang

; =============================================================================
; Load configuration
; =============================================================================
load_config:
    pusha

    mov si, msg_loadcfg
    call print_string

    ; Reset disk
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    ; Read config sector
    mov bx, config_buffer
    mov ah, 0x02
    mov al, 2
    mov ch, 0
    mov cl, 2
    mov dh, 1
    mov dl, [boot_drive]
    int 0x13

    jc .error

    call parse_config

    cmp byte [entry_count], 0
    je .setup_defaults

    popa
    ret

.error:
.setup_defaults:
    mov byte [entry_count], 1
    mov si, msg_defaults
    call print_string
    popa
    ret

; =============================================================================
; Main menu loop
; =============================================================================
main_loop:
    pusha

    call draw_text_menu

.input:
    call read_key

    cmp ah, 0x48
    je .up
    cmp ah, 0x50
    je .down
    cmp al, 13
    je .select

    jmp .input

.up:
    cmp byte [menu_selection], 0
    je .input
    dec byte [menu_selection]
    jmp .input

.down:
    movzx ax, byte [menu_selection]
    inc ax
    cmp al, [entry_count]
    jae .input
    mov [menu_selection], al
    jmp .input

.select:
    call boot_kernel
    jmp .input

    popa
    ret

; =============================================================================
; Draw text menu
; =============================================================================
draw_text_menu:
    pusha

    call clear_screen

    mov si, msg_menu
    call print_string

    movzx cx, byte [entry_count]
    xor bh, bh
    movzx bx, byte [menu_selection]

.loop:
    push cx
    mov ax, bx
    mov cx, 1
    sub cx, ax
    je .selected

    mov si, .space
    call print_string
    jmp .print_entry

.selected:
    mov si, .arrow
    call print_string

.print_entry:
    mov si, .entry_name
    call print_string
    call print_newline

    pop cx
    loop .loop

    popa
    ret

.space: db "  ", 0
.arrow: db "> ", 0
.entry_name: db "DIT-OS", 0

; =============================================================================
; Boot kernel
; =============================================================================
boot_kernel:
    pusha

    mov si, msg_loading
    call print_string

    ; Load kernel from fixed location
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

    mov si, msg_jump
    call print_string

    popa

    ; Jump to kernel
    mov dl, [boot_drive]
    mov bx, 0x1000
    push bx
    mov ax, 0x0000
    push ax
    retf

.error:
    mov si, msg_error
    call print_string
    popa
    ret

; =============================================================================
; Messages
; =============================================================================
msg_stage2_start: db "S2START", 13, 10, 0
msg_stage2: db "DIT-OS Stage2...", 13, 10, 0
msg_loadcfg: db "Loading config...", 13, 10, 0
msg_defaults: db "Using defaults", 13, 10, 0
msg_init_menu: db "Initializing menu...", 13, 10, 0
msg_menu: db "=== Boot Menu ===", 13, 10, 0
msg_loading: db "Loading kernel...", 13, 10, 0
msg_jump: db "Booting...", 13, 10, 0
msg_error: db "Error!", 13, 10, 0
