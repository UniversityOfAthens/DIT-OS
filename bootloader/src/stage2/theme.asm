; =============================================================================
; DIT-OS Stage 2 - Theme System
; Phase 5: Theme Engine
; =============================================================================

; External functions
extern clear_graphics_screen
extern draw_rect
extern draw_string
extern draw_char
extern plot_pixel

; External data
extern menu_selection
extern entries
extern entry_count
extern graphics_mode
extern framebuffer_addr

; Note: Functions from graphics.asm, font.asm, fat32.asm, bmp.asm are included in stage2.asm
; menu_selection is defined in stage2.asm

; =============================================================================
; Data section - Theme configuration
; =============================================================================
SECTION .data

; Theme configuration structure (embedded directly)
theme_screen_width:      dw 800
theme_screen_height:     dw 600
theme_bg_color:         dd 0xFF103000
theme_header_height:    dw 60
theme_header_color:     dd 0xFF004080
theme_title_text_ptr:   dd default_title
theme_title_x:          dw 300
theme_title_y:          dw 18
theme_title_color:       dd 0xFFFFFFFF
theme_menu_x:           dw 100
theme_menu_y:           dw 120
theme_menu_width:       dw 600
theme_menu_height:       dw 320
theme_menu_bg_color:    dd 0xFF002040
theme_menu_border:      dw 1
theme_menu_border_color: dd 0xFF0060A0
theme_item_height:      dw 40
theme_item_font_size:   dw 24
theme_item_color:       dd 0xFFCCCCCC
theme_item_sel_color:   dd 0xFFFFFFFF
theme_item_sel_bg:      dd 0xFF0060C0
theme_item_arrow_color: dd 0xFF00A0FF
theme_footer_height:    dw 60
theme_footer_color:     dd 0xFF001830
theme_footer_text_ptr:  dd default_footer
theme_timeout_bar:      db 1
theme_timeout_color:    dd 0xFF00FF00
theme_timeout_height:   dw 4

; Theme file path
theme_file_path: db "/boot/bootloader/themes/dit/theme.txt", 0

; Background image path
theme_bg_image_path: db "/boot/bootloader/themes/dit/background.bmp", 0
theme_use_bg_image:  db 0                    ; Set to 1 to enable background image

default_title:  db "DIT-OS", 0
default_footer: db "Arrow keys: select | Enter: boot | 1-9: quick select", 0

theme_loaded:   db 0
theme_file_loaded: db 0

; =============================================================================
; Load theme from file
; Input: ESI = file path string
; =============================================================================
global load_theme_from_file
load_theme_from_file:
    pusha
    
    ; Try to load the theme file
    ; This would use the filesystem to read the file
    ; For now, just use defaults
    
    ; Try to load background image
    mov esi, theme_bg_image_path
    call try_load_background_image
    
    ; Mark theme as loaded from file
    mov byte [theme_file_loaded], 1
    
    popad
    ret

; =============================================================================
; Load background image
; Input: ESI = image file path
; Note: Requires FAT32 driver and file system to be implemented
; =============================================================================
global load_background_image
load_background_image:
    pushad
    
    ; FAT32 filesystem not available in current build
    ; Would require proper linker setup
    
    stc
    popad
    ret

; =============================================================================
; Try to load background image
; Input: ESI = image path
; =============================================================================
try_load_background_image:
    pushad
    
    ; Background image loading requires:
    ; 1. FAT32 filesystem driver (fat32.asm - available but not linked)
    ; 2. File system initialization
    ; 3. BMP/PNG decoder integration
    
    ; For now, skip
    popad
    ret

; =============================================================================
; Load theme from GRUB-style config
; Parse a simple key=value theme file
; Input: ESI = theme data, ECX = size
; =============================================================================
global parse_theme_config
parse_theme_config:
    pusha
    
    mov [theme_data_ptr], esi
    mov [theme_data_size], ecx
    
.parse_loop:
    ; Skip whitespace
.skip_ws:
    lodsb
    dec ecx
    cmp al, ' '
    je .skip_ws
    cmp al, 9       ; Tab
    je .skip_ws
    cmp al, 10      ; LF
    je .skip_ws
    cmp al, 13      ; CR
    je .skip_ws
    test ecx, ecx
    jz .done
    
    ; Check for comment
    cmp al, '#'
    je .skip_line
    
    ; Save start of key
    dec esi
    inc ecx
    mov edi, parse_buffer
    
.key_loop:
    lodsb
    dec ecx
    cmp al, '='
    je .found_eq
    cmp al, ' '
    je .found_eq
    cmp al, 9
    je .found_eq
    stosb
    jmp .key_loop
    
.found_eq:
    mov byte [edi], 0
    
    ; Skip whitespace after =
.skip_ws2:
    lodsb
    dec ecx
    cmp al, ' '
    je .skip_ws2
    cmp al, 9
    je .skip_ws2
    
    ; Now at value
    dec esi
    inc ecx
    
    ; Parse based on key
    mov edi, parse_buffer
    
    ; Check each key
    call check_theme_key
    jc .parsed
    
    ; Skip to next line
.skip_line:
    test ecx, ecx
    jz .done
    lodsb
    dec ecx
    cmp al, 10
    jne .skip_line
    
    jmp .parse_loop
    
.parsed:
    jmp .skip_line
    
.done:
    popad
    ret

; =============================================================================
; Data for theme file parser
; =============================================================================
theme_data_ptr:     dd 0
theme_data_size:    dd 0
parse_buffer:       times 64 db 0


; Check and parse theme keys
check_theme_key:
    push edi
    
    ; BG_COLOR
    mov edi, parse_buffer
    cmp byte [edi], 'B'
    jne .not_bg
    cmp dword [edi+1], 'G_CO'
    jne .not_bg
    pop edi
    call parse_hex_color_theme
    mov [theme_bg_color], eax
    stc
    ret
    
.not_bg:
    ; HEADER_COLOR
    cmp dword [edi], 'HEAD'
    jne .not_header
    cmp dword [edi+4], 'ER_C'
    jne .not_header
    pop edi
    call parse_hex_color_theme
    mov [theme_header_color], eax
    stc
    ret
    
.not_header:
    ; TITLE_TEXT
    cmp dword [edi], 'TITL'
    jne .not_title
    cmp byte [edi+4], 'E'
    jne .not_title
    pop edi
    ; Save string pointer
    mov [theme_title_text_ptr], esi
    stc
    ret
    
.not_title:
    ; MENU_BG_COLOR
    cmp dword [edi], 'MENU'
    jne .not_menu
    cmp dword [edi+4], '_BG_'
    jne .not_menu
    pop edi
    call parse_hex_color_theme
    mov [theme_menu_bg_color], eax
    stc
    ret
    
.not_menu:
    pop edi
    clc
    ret

; Parse hex color for theme
parse_hex_color_theme:
    push ebx
    xor ebx, ebx
    
    ; Skip 0x or 0X
    cmp byte [esi], '0'
    jne .no_px
    cmp byte [esi+1], 'x'
    je .has_px
    cmp byte [esi+1], 'X'
    jne .no_px
    
.has_px:
    add esi, 2
    
.no_px:
    ; Parse hex digits
.hex_loop:
    test ecx, ecx
    jz .done
    
    mov al, [esi]
    inc esi
    dec ecx
    
    cmp al, '0'
    jb .done
    cmp al, '9'
    jbe .digit
    cmp al, 'A'
    jb .done
    cmp al, 'F'
    jbe .upper
    cmp al, 'a'
    jb .done
    cmp al, 'f'
    ja .done
    
    sub al, 0x20
    
.upper:
    sub al, 'A' - 10
    jmp .add_digit
    
.digit:
    sub al, '0'
    
.add_digit:
    shl ebx, 4
    or bl, al
    jmp .hex_loop
    
.done:
    ; Convert to ARGB (assume RRGGBB -> AABBGGRR)
    shl ebx, 8
    or ebx, 0xFF
    mov eax, ebx
    
    pop ebx
    ret

; =============================================================================
; Load default theme
; =============================================================================
global load_theme
load_theme:
    pusha
    
    ; Already have defaults in data section
    mov byte [theme_loaded], 1
    
    popad
    ret

; =============================================================================
; Render theme background
; =============================================================================
global render_theme_background
render_theme_background:
    pusha
    
    ; Check if we should load background image
    cmp byte [theme_use_bg_image], 1
    jne .color_background
    
    ; Try to load background image
    mov esi, theme_bg_image_path
    call load_background_image
    jc .color_background
    
    ; Successfully loaded background image
    jmp .done
    
.color_background:
    ; Clear screen with background color
    mov eax, [theme_bg_color]
    call clear_graphics_screen
    
    ; Draw header bar
    mov ecx, 0
    mov edx, 0
    movzx esi, word [theme_screen_width]
    movzx edi, word [theme_header_height]
    mov eax, [theme_header_color]
    call draw_rect
    
    ; Draw menu background
    movzx ecx, word [theme_menu_x]
    movzx edx, word [theme_menu_y]
    movzx esi, word [theme_menu_width]
    movzx edi, word [theme_menu_height]
    mov eax, [theme_menu_bg_color]
    call draw_rect
    
    ; Draw menu border (top)
    movzx ecx, word [theme_menu_x]
    movzx edx, word [theme_menu_y]
    movzx esi, word [theme_menu_width]
    mov edi, 1              ; Border height
    mov eax, [theme_menu_border_color]
    call draw_rect
    
    ; Draw menu border (bottom)
    movzx ecx, word [theme_menu_x]
    movzx edx, word [theme_menu_y]
    movzx eax, word [theme_menu_height]
    add edx, eax
    sub edx, 1
    movzx esi, word [theme_menu_width]
    mov edi, 1
    mov eax, [theme_menu_border_color]
    call draw_rect
    
    ; Draw menu border (left)
    movzx ecx, word [theme_menu_x]
    movzx edx, word [theme_menu_y]
    mov esi, 1
    movzx edi, word [theme_menu_height]
    mov eax, [theme_menu_border_color]
    call draw_rect
    
    ; Draw menu border (right)
    movzx ecx, word [theme_menu_x]
    movzx eax, word [theme_menu_width]
    add ecx, eax
    sub ecx, 1
    movzx edx, word [theme_menu_y]
    mov esi, 1
    movzx edi, word [theme_menu_height]
    mov eax, [theme_menu_border_color]
    call draw_rect
    
.done:
    popa
    ret

; =============================================================================
; Render theme title
; =============================================================================
global render_theme_title
render_theme_title:
    pusha
    
    mov esi, [theme_title_text_ptr]
    test esi, esi
    jz .done
    
    movzx ecx, word [theme_title_x]
    movzx edx, word [theme_title_y]
    mov ebx, [theme_title_color]
    
    call draw_string
    
.done:
    popa
    ret

; =============================================================================
; Render theme footer
; =============================================================================
global render_theme_footer
render_theme_footer:
    pusha
    
    ; Draw footer bar
    mov ecx, 0
    movzx edx, word [theme_screen_height]
    movzx eax, word [theme_footer_height]
    sub edx, eax
    movzx esi, word [theme_screen_width]
    movzx edi, word [theme_footer_height]
    mov eax, [theme_footer_color]
    call draw_rect
    
    ; Draw footer text
    mov esi, [theme_footer_text_ptr]
    test esi, esi
    jz .done
    
    mov ecx, 100
    movzx edx, word [theme_screen_height]
    sub edx, 30
    mov ebx, 0xFF888888
    
    call draw_string
    
.done:
    popa
    ret

; =============================================================================
; Render menu item with theme styling
; Input: ESI = entry name, EBX = item index
; =============================================================================
global render_theme_menu_item
render_theme_menu_item:
    pusha
    
    ; Calculate Y position
    movzx eax, word [theme_item_height]
    mul bx
    add eax, [theme_menu_y]
    mov edx, eax
    
    ; Check if selected
    cmp bl, [menu_selection]
    jne .not_selected
    
    ; Draw selection highlight background
    push ebx
    push esi
    
    movzx ecx, word [theme_menu_x]
    add ecx, 10
    movzx esi, word [theme_menu_width]
    sub esi, 20
    movzx edi, word [theme_item_height]
    sub edi, 10
    mov eax, [theme_item_sel_bg]
    call draw_rect
    
    pop esi
    pop ebx
    
    ; Use selected colors
    mov ebx, [theme_item_sel_color]
    jmp .draw_text
    
.not_selected:
    mov ebx, [theme_item_color]
    
.draw_text:
    push ebx
    movzx ecx, word [theme_menu_x]
    add ecx, 40
    movzx edx, word [theme_item_height]
    mul bx
    add edx, [theme_menu_y]
    add edx, 10
    pop ebx
    
    call draw_string
    
    ; Draw selection arrow for selected item
    cmp bl, [menu_selection]
    jne .done
    
    push esi
    movzx ecx, word [theme_menu_x]
    add ecx, 20
    movzx edx, word [theme_item_height]
    mul bx
    add edx, [theme_menu_y]
    add edx, 12
    mov esi, [theme_item_arrow_color]
    mov al, '>'
    call draw_char
    pop esi
    
.done:
    popa
    ret

; =============================================================================
; Render timeout bar
; Input: EBX = current timeout value, ECX = max timeout
; =============================================================================
global render_timeout_bar
render_timeout_bar:
    pusha
    
    cmp byte [theme_timeout_bar], 0
    je .done
    
    test ebx, ebx
    jz .done
    test ecx, ecx
    jz .done
    
    ; Calculate bar width (ebx * menu_width / ecx)
    movzx eax, word [theme_menu_width]
    mul ebx
    div ecx
    movzx esi, ax
    
    ; Draw timeout bar at bottom of menu
    movzx ecx, word [theme_menu_x]
    movzx edx, word [theme_menu_y]
    movzx eax, word [theme_menu_height]
    add edx, eax
    movzx eax, word [theme_timeout_height]
    sub edx, eax
    sub edx, 5
    movzx edi, word [theme_timeout_height]
    mov eax, [theme_timeout_color]
    call draw_rect
    
.done:
    popa
    ret
