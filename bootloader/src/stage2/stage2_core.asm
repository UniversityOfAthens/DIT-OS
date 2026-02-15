; =============================================================================
; DIT-OS Stage 2 - Core Functions (ELF Build)
; This file provides all core functions needed by other modules
; =============================================================================
%include "constants.inc"

SECTION .text

; =============================================================================
; Core I/O Functions
; =============================================================================

GLOBAL print_string
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

GLOBAL print_char
print_char:
    pusha
    mov ah, 0x0E
    int 0x10
    popa
    ret

GLOBAL print_newline
print_newline:
    pusha
    mov ah, 0x0E
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    popa
    ret

GLOBAL read_key
read_key:
    xor ah, ah
    int 0x16
    ret

GLOBAL hang
hang:
    hlt
    jmp hang

GLOBAL clear_screen
clear_screen:
    pusha
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    popa
    ret

GLOBAL set_cursor
set_cursor:
    pusha
    xor bh, bh
    mov ah, 0x02
    int 0x10
    popa
    ret

; =============================================================================
; Disk I/O Functions  
; =============================================================================

GLOBAL read_sectors
read_sectors:
    pusha
    mov si, .retry_msg
    call print_string
    popa
    
.retry:
    pusha
    mov ah, 0x02
    int 0x13
    jnc .success
    
    dec dx
    jnz .retry
    
    call .error
    popa
    stc
    ret
    
.success:
    popa
    clc
    ret

.error:
    popa
    mov si, .err_msg
    call print_string
    call hang
    ret

.retry_msg: db "Reading sectors...", 13, 10, 0
.err_msg: db "Disk error!", 13, 10, 0

; =============================================================================
; Graphics Functions
; =============================================================================

GLOBAL plot_pixel
plot_pixel:
    pusha
    mov edi, [framebuffer_addr]
    movzx eax, word [screen_width]
    mul edx
    add eax, ecx
    movzx ebx, byte [bits_per_pixel]
    shr ebx, 3
    mul ebx
    add edi, eax
    
    mov [edi], bx
    popa
    ret

GLOBAL clear_graphics_screen
clear_graphics_screen:
    pusha
    mov edi, [framebuffer_addr]
    movzx ecx, word [screen_width]
    movzx eax, word [screen_height]
    mul ecx
    movzx ebx, byte [bits_per_pixel]
    shr ebx, 3
    mul ebx
    xchg ecx, eax
    
    mov ah, [bg_color]
    mov al, [bg_color+1]
    mov bx, ax
    rep stosw
    popa
    ret

GLOBAL draw_rect
draw_rect:
    ret

; Global variables for draw_rect
height: dw 0
width: dw 0
row_stride: dd 0
bg_color: dd 0xFF101000

GLOBAL graphics_available
graphics_available: db 0
GLOBAL graphics_mode
graphics_mode: dw 0
GLOBAL framebuffer_addr
framebuffer_addr: dd 0
GLOBAL screen_width
screen_width: dw 800
GLOBAL screen_height
screen_height: dw 600
GLOBAL bits_per_pixel
bits_per_pixel: db 32

GLOBAL check_vesa
check_vesa:
    pusha
    mov ax, 0x4F00
    mov di, vbe_info_block
    int 0x10
    cmp ax, 0x004F
    jne .not_available
    
    mov si, vbe_info_block
    cmp dword [si], 0x32454256
    jne .not_available
    
    mov byte [graphics_available], 1
    mov si, .msg_ok
    call print_string
    popa
    ret

.not_available:
    mov byte [graphics_available], 0
    mov si, .msg_fail
    call print_string
    popa
    ret

.msg_ok: db "VESA available", 13, 10, 0
.msg_fail: db "VESA not available", 13, 10, 0

vbe_info_block: times 512 db 0
vbe_mode_info: times 256 db 0

GLOBAL set_graphics_mode
set_graphics_mode:
    pusha
    mov ax, 0x4F02
    mov bx, 0x4101
    mov di, vbe_mode_info
    int 0x10
    cmp ax, 0x004F
    jne .fail
    
    movzx eax, word [vbe_mode_info+16]
    mov [framebuffer_addr], eax
    movzx eax, word [vbe_mode_info+18]
    mov [screen_width], eax
    movzx eax, word [vbe_mode_info+20]
    mov [screen_height], eax
    mov al, [vbe_mode_info+25]
    mov [bits_per_pixel], al
    
    mov si, .msg_ok
    call print_string
    popa
    ret

.fail:
    mov si, .msg_fail
    call print_string
    popa
    ret

.msg_ok: db "Graphics mode set", 13, 10, 0
.msg_fail: db "Failed to set graphics mode", 13, 10, 0

; =============================================================================
; Menu Functions
; =============================================================================

GLOBAL init_menu
init_menu:
    ret

GLOBAL draw_graphics_menu
draw_graphics_menu:
    pusha
    call clear_graphics_screen
    
    mov si, .msg
    call print_string
    popa
    ret

.msg: db "Boot Menu", 13, 10, 0

GLOBAL parse_config
parse_config:
    pusha
    mov si, config_buffer
    mov byte [entry_count], 0
    popa
    ret

; =============================================================================
; Font Functions
; =============================================================================

GLOBAL draw_char
draw_char:
    ret

GLOBAL draw_string
draw_string:
    ret

GLOBAL string_width
string_width:
    xor eax, eax
    ret

GLOBAL font_color
font_color: dd 0xFFFFFFFF

font_data:
    db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    db 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    times 128 db 0

; =============================================================================
; Filesystem Functions (stub implementations)
; =============================================================================

GLOBAL fat32_init
fat32_init:
    stc
    ret

GLOBAL fat32_open_file
fat32_open_file:
    xor eax, eax
    ret

GLOBAL fat32_read_file
fat32_read_file:
    xor ecx, ecx
    ret

GLOBAL fat32_initialized
fat32_initialized: db 0
GLOBAL partition_start
partition_start: dd 0

GLOBAL load_theme_from_file
load_theme_from_file:
    ret

GLOBAL load_background_image
load_background_image:
    ret

GLOBAL parse_theme_config
parse_theme_config:
    ret

GLOBAL load_theme
load_theme:
    ret

GLOBAL render_theme_background
render_theme_background:
    ret

GLOBAL render_theme_title
render_theme_title:
    ret

GLOBAL render_theme_menu_item
render_theme_menu_item:
    ret

GLOBAL render_theme_footer
render_theme_footer:
    ret

GLOBAL bmp_load_image
bmp_load_image:
    stc
    ret

GLOBAL render_bmp_background
render_bmp_background:
    ret

GLOBAL pf2_load_font
pf2_load_font:
    stc
    ret

GLOBAL pf2_draw_char
pf2_draw_char:
    ret

GLOBAL pf2_font_loaded
pf2_font_loaded: db 0

; Mouse support
GLOBAL mouse_init
GLOBAL mouse_available
GLOBAL mouse_x
GLOBAL mouse_y
GLOBAL mouse_buttons
GLOBAL mouse_get_position
GLOBAL mouse_enable
GLOBAL mouse_disable
GLOBAL mouse_poll

mouse_available: db 0
mouse_x: dw 320
mouse_y: dw 240
mouse_buttons: db 0

; =============================================================================
; Menu Data
; =============================================================================

GLOBAL entry_count
entry_count: db 0
GLOBAL menu_selection
menu_selection: db 0
GLOBAL entries
entries: times 256 db 0
ENTRY_SIZE equ 128

; =============================================================================
; Configuration
; =============================================================================

GLOBAL force_graphics
force_graphics: db 0
GLOBAL force_text
force_text: db 0
GLOBAL use_text_mode
use_text_mode: db 0
GLOBAL should_use_graphics
should_use_graphics: db 0
GLOBAL current_mode
current_mode: db 0

; =============================================================================
; Boot data
; =============================================================================

GLOBAL boot_drive
boot_drive: db 0

; =============================================================================
; Buffers
; =============================================================================

GLOBAL config_buffer
config_buffer: times 2048 db 0
