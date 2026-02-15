; =============================================================================
; DIT-OS Stage 2 - VESA/VBE Graphics Support
; Phase 3: Graphics Foundation
; =============================================================================

; Note: For flat binary build, we cannot use extern
; Labels defined in other modules must be declared differently

; VBE Info Block structure
struc VbeInfoBlock
    .signature      resb 4      ; 'VESA'
    .version        resw 1      ; BCD version
    .oemStringPtr   resd 1      ; Far pointer to OEM string
    .capabilities   resd 1      ; Capabilities flags
    .videoModesPtr  resd 1      ; Far pointer to video mode list
    .totalMemory    resw 1      ; Memory in 64KB blocks
    .reserved       resb 492     ; Reserved
endstruc

; VBE Mode Info structure
struc VbeModeInfo
    .attributes     resw 1
    .windowA        resb 1
    .windowB        resb 1
    .granularity    resw 1
    .windowSize     resw 1
    .segmentA       resw 1
    .segmentB       resw 1
    .winFuncPtr     resd 1
    .pitch          resw 1
    .width          resw 1
    .height         resw 1
    .wChar          resb 1
    .yChar          resb 1
    .planes         resb 1
    .bpp            resb 1
    .banks          resb 1
    .memoryModel    resb 1
    .bankSize       resb 1
    .imagePages     resb 1
    .reserved0      resb 1
    .redMaskSize    resb 1
    .redFieldPos    resb 1
    .greenMaskSize  resb 1
    .greenFieldPos  resb 1
    .blueMaskSize   resb 1
    .blueFieldPos   resb 1
    .rsvMaskSize    resb 1
    .rsvFieldPos    resb 1
    .directColorInfo resb 1
    .framebuffer    resd 1      ; Linear framebuffer address
    .reserved1      resb 212
endstruc

; -----------------------------------------------------------------------------
; Check for VBE support and store info
; Returns: CF set if VBE not available, CF clear if VBE available
; -----------------------------------------------------------------------------
global check_vesa
check_vesa:
    pusha
    push es
    
    ; Set up buffer for VBE info
    mov ax, 0x1000          ; Segment where we'll store VBE info
    mov es, ax
    xor di, di
    
    ; Fill signature 'VBE2' to indicate VBE 2.0+ support
    mov dword [es:di], 'VBE2'
    
    ; Call VBE function 00h - Return VBE Controller Information
    mov ax, 0x4F00
    int 0x10
    
    ; Check if VBE is present (AH should be 0)
    cmp ax, 0x004F
    jne .no_vbe
    
    ; Verify signature 'VESA'
    cmp dword [es:di], 'VESA'
    jne .no_vbe
    
    ; VBE is available, store that we have graphics
    mov byte [graphics_available], 1
    
    pop es
    popa
    clc
    ret
    
.no_vbe:
    mov byte [graphics_available], 0
    pop es
    popa
    stc
    ret

; -----------------------------------------------------------------------------
; Find a suitable graphics mode (800x600x32bpp)
; Returns: AX = mode number, or 0 if not found
; -----------------------------------------------------------------------------
global find_graphics_mode
find_graphics_mode:
    pusha
    push es
    
    ; Get pointer to video mode list from VBE info
    mov ax, 0x1000
    mov es, ax
    mov si, [es:VbeInfoBlock.videoModesPtr]
    mov ax, [es:VbeInfoBlock.videoModesPtr + 2]
    mov es, ax
    
.search_loop:
    lodsw
    cmp ax, 0xFFFF          ; End of mode list
    je .not_found
    
    mov cx, ax              ; Save mode number
    
    ; Get mode info
    push es
    push si
    
    mov ax, 0x2000          ; Buffer for mode info
    mov es, ax
    xor di, di
    
    mov ax, 0x4F01          ; Get VBE Mode Info
    int 0x10
    
    cmp ax, 0x004F
    jne .next_mode
    
    ; Check if mode supports linear framebuffer
    mov ax, [es:VbeModeInfo.attributes]
    test ax, 0x0080         ; Linear framebuffer bit
    jz .next_mode
    
    ; Check dimensions and bpp
    mov ax, [es:VbeModeInfo.width]
    cmp ax, 800
    jne .next_mode
    
    mov ax, [es:VbeModeInfo.height]
    cmp ax, 600
    jne .next_mode
    
    mov al, [es:VbeModeInfo.bpp]
    cmp al, 32
    jne .next_mode
    
    ; Found suitable mode!
    mov [graphics_mode], cx
    mov eax, [es:VbeModeInfo.framebuffer]
    mov [framebuffer_addr], eax
    
    pop si
    pop es
    pop es
    popa
    mov ax, cx
    ret
    
.next_mode:
    pop si
    pop es
    jmp .search_loop
    
.not_found:
    pop es
    popa
    xor ax, ax
    ret

; -----------------------------------------------------------------------------
; Set graphics mode
; Input: AX = mode number
; -----------------------------------------------------------------------------
global set_graphics_mode
set_graphics_mode:
    pusha
    
    ; Set VBE mode with linear framebuffer
    or ax, 0x4000           ; Enable linear framebuffer
    mov bx, ax
    mov ax, 0x4F02
    int 0x10
    
    popa
    ret

; -----------------------------------------------------------------------------
; Plot pixel in graphics mode
; Input: CX = x, DX = y, EAX = color (ARGB)
; -----------------------------------------------------------------------------
global plot_pixel
plot_pixel:
    pusha
    
    ; Calculate offset: (y * width + x) * 4
    mov ebx, [framebuffer_addr]
    
    mov eax, edx
    mov edx, 800            ; Width
    mul edx
    add eax, ecx
    shl eax, 2              ; * 4 bytes per pixel
    
    add ebx, eax
    
    pop eax                 ; Restore color
    mov [ebx], eax          ; Write pixel
    
    popa
    ret

; -----------------------------------------------------------------------------
; Clear screen in graphics mode (fill with color)
; Input: EAX = color
; -----------------------------------------------------------------------------
global clear_graphics_screen
clear_graphics_screen:
    pusha
    
    mov edi, [framebuffer_addr]
    mov ecx, 800 * 600      ; Total pixels
    
.clear_loop:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clear_loop
    
    popa
    ret

; -----------------------------------------------------------------------------
; Data section
; -----------------------------------------------------------------------------
SECTION .data

global graphics_available
graphics_available:     db 0
global graphics_mode
graphics_mode:          dw 0
global framebuffer_addr
framebuffer_addr:       dd 0

; -----------------------------------------------------------------------------
; Config option for forcing text mode
; -----------------------------------------------------------------------------
global use_text_mode
use_text_mode:          db 0

; -----------------------------------------------------------------------------
; Check if we should use graphics or text mode
; Returns: AL = 0 for text mode, 1 for graphics mode
; -----------------------------------------------------------------------------
global should_use_graphics
should_use_graphics:
    push bx
    
    ; Check if text mode is forced
    cmp byte [use_text_mode], 1
    je .text_mode
    
    ; Check if VBE is available
    cmp byte [graphics_available], 1
    jne .text_mode
    
    ; Check if we found a valid mode
    cmp word [graphics_mode], 0
    je .text_mode
    
    pop bx
    mov al, 1
    ret
    
.text_mode:
    pop bx
    xor al, al
    ret

%include "font.asm"

; -----------------------------------------------------------------------------
; Draw menu in graphics mode using theme system
; -----------------------------------------------------------------------------
global draw_graphics_menu
draw_graphics_menu:
    pusha
    
    ; Load theme (uses defaults if no theme file)
    call load_theme
    
    ; Render theme background (header, menu bg, borders)
    call render_theme_background
    
    ; Render theme title
    call render_theme_title
    
    ; Draw menu entries using theme
    xor bl, bl              ; Entry index
    
.entry_loop:
    cmp bl, [entry_count]
    jae .entries_done
    
    ; Get entry name
    push ebx
    xor eax, eax
    mov al, bl
    mov cx, 224              ; ENTRY_SIZE
    mul cx
    mov esi, entries
    add esi, eax
    pop ebx
    
    ; Render this menu item with theme
    call render_theme_menu_item
    
    inc bl
    jmp .entry_loop
    
.entries_done:
    ; Render theme footer
    call render_theme_footer
    
    popa
    ret

msg_graphics_title:   db "DIT-OS Boot Manager", 0
msg_graphics_footer:  db "Arrow keys: select | Enter: boot | 1-9: quick select", 0
msg_graphics_timeout: db "Auto-boot in ", 0

; -----------------------------------------------------------------------------
; Draw filled rectangle
; Input: ECX = x, EDX = y, ESI = width, EDI = height, EAX = color
; -----------------------------------------------------------------------------
draw_rect:
    pusha
    
    mov ebx, [framebuffer_addr]
    
    ; Calculate starting address
    push eax
    mov eax, edx
    mov edx, 800
    mul edx
    add eax, ecx
    shl eax, 2
    add ebx, eax
    pop eax
    
    mov ecx, esi            ; Save width
    
.row_loop:
    push ecx
    mov ecx, esi            ; Width counter
    
.pixel_loop:
    mov [ebx], eax
    add ebx, 4
    dec ecx
    jnz .pixel_loop
    
    pop ecx
    ; Move to next row
    mov edx, 800
    sub edx, esi
    shl edx, 2
    add ebx, edx
    
    dec edi
    jnz .row_loop
    
    popa
    ret
