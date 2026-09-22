; =============================================================================
; DIT-OS Stage 2 - BMP Image Decoder
; For background images in bootloader
; =============================================================================

; External symbols
extern framebuffer_addr
extern plot_pixel

; =============================================================================
; Load and decode BMP image
; Input: ESI = pointer to BMP data
; Output: EAX = framebuffer address, CF = error
; =============================================================================
global bmp_load_image
bmp_load_image:
    pushad

    ; Check BMP signature ('BM' = 0x4D42)
    lodsw
    cmp ax, 0x4D42
    jne .error

    ; Skip file size (4 bytes)
    lodsd

    ; Skip reserved bytes (4 bytes)
    lodsd

    ; Get pixel data offset
    lodsd
    mov [bmp_data_offset], eax

    ; Get DIB header size
    lodsd
    mov [bmp_header_size], eax

    ; Check if this is BITMAPINFOHEADER (40 bytes)
    cmp eax, 40
    je .standard_header

    ; Skip to standard header
    sub eax, 40
    add esi, eax

.standard_header:
    ; Get width
    lodsd
    mov [bmp_width], eax

    ; Get height (can be negative for top-down)
    lodsd
    mov [bmp_height], eax

    ; Get color planes
    lodsw

    ; Get bits per pixel
    lodsw
    mov [bmp_bpp], ax

    ; Skip compression info
    lodsd

    ; Get image size
    lodsd
    mov [bmp_image_size], eax

    ; Skip remaining header data
    add esi, 20

    ; Point to pixel data
    movzx esi, word [bmp_data_offset]
    add esi, [bmp_file_start]

    ; Get framebuffer
    mov edi, [framebuffer_addr]

    ; Check if height is negative (top-down)
    cmp dword [bmp_height], 0
    jl .top_down

    ; Bottom-up BMP: flip the image
    neg dword [bmp_height]
    movzx eax, word [bmp_width]
    mul dword [bmp_height]           ; Total scanlines
    movzx ebx, word [bmp_bpp]
    shr ebx, 3                       ; Bytes per pixel
    mul ebx
    add esi, eax                      ; Start from last row

.bottom_up:
    ; Each row is 4-byte aligned
    movzx ecx, word [bmp_width]
    movzx ebx, word [bmp_bpp]
    shr ebx, 3
    mul ebx                          ; Row size

    ; Align to 4 bytes
    add eax, 3
    and eax, ~3

    push ecx
    mov ecx, eax
    sub esi, ecx                     ; Go to previous row
    pop ecx

    ; Copy row (BGR -> RGB)
    call bmp_copy_row

    dec dword [bmp_height]
    jnz .bottom_up
    jmp .done

.top_down:
    ; Top-down BMP: copy directly
    movzx ecx, word [bmp_width]
    call bmp_copy_row

    dec dword [bmp_height]
    jnz .top_down

.done:
    clc
    popad
    ret

.error:
    stc
    popad
    ret

; =============================================================================
; Copy one BMP row to framebuffer
; Input: ESI = source, EDI = dest, ECX = width in pixels
; =============================================================================
bmp_copy_row:
    pushad

.row_loop:
    test cx, cx
    jz .done

    ; Read BGR pixel
    lodsb                           ; Blue
    mov bh, al
    lodsb                           ; Green
    mov bl, al
    lodsb                           ; Red
    mov ah, bl
    mov al, bh

    ; Convert BGR to ARGB
    shl eax, 8
    or eax, 0xFF000000

    ; Write to framebuffer
    mov [edi], eax
    add edi, 4

    dec cx
    jnz .row_loop

.done:
    popad
    ret

; =============================================================================
; Data section
; =============================================================================
SECTION .data

bmp_file_start:    dd 0
bmp_data_offset:  dd 0
bmp_header_size:  dd 0
bmp_width:        dd 0
bmp_height:       dd 0
bmp_bpp:          dw 0
bmp_image_size:   dd 0

; =============================================================================
; Render BMP as background
; Input: ESI = BMP file data
; =============================================================================
global render_bmp_background
render_bmp_background:
    pushad

    ; Save start of file
    mov [bmp_file_start], esi

    ; Load the BMP
    call bmp_load_image

    popad
    ret
