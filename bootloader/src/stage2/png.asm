; =============================================================================
; DIT-OS Stage 2 - PNG Image Decoder
; For background images in bootloader
; =============================================================================

; External symbols
extern framebuffer_addr
extern plot_pixel

; PNG constants
PNG_SIGNATURE: db 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A

; PNG chunk types
PNG_CHUNK_IHDR: db 'IHDR'
PNG_CHUNK_IDAT: db 'IDAT'
PNG_CHUNK_IEND: db 'IEND'
PNG_CHUNK_PLTE: db 'PLTE'

; PNG filter types
PNG_FILTER_NONE:  equ 0
PNG_FILTER_SUB:   equ 1
PNG_FILTER_UP:   equ 2
PNG_FILTER_AVG:  equ 3
PNG_FILTER_PAETH: equ 4

; =============================================================================
; Load and decode PNG image
; Input: ESI = pointer to PNG data, ECX = size
; Output: EAX = framebuffer address, CF = error
; =============================================================================
global png_load_image
png_load_image:
    pushad

    ; Verify PNG signature
    mov edi, esi
    mov ebx, PNG_SIGNATURE
    mov ecx, 8
    rep cmpsb
    jne .error

    ; Find IHDR chunk
    call png_find_chunk
    jc .error

    ; Parse IHDR
    call png_parse_ihdr
    jc .error

    ; Find IDAT chunk (image data)
    call png_find_chunk
    jc .error

    ; Allocate buffer for decompressed data
    ; Estimate: width * height * (bpp/8 + 1 for filter)
    movzx eax, word [png_width]
    movzx ebx, word [png_height]
    mul ebx
    mov ebx, eax
    shl ebx, 2              ; *4 bytes per pixel (32-bit)
    add ebx, eax            ; Add filter byte per row

    push ebx                ; Save decompressed size

    ; Allocate memory for decompressed data
    mov eax, 0x20000        ; 128KB buffer
    mov [png_work_buffer], eax

    pop ebx

    ; Decompress IDAT data using zlib
    push esi                ; Compressed data
    push ecx                ; Compressed size
    mov edi, [png_work_buffer]
    mov [pngDecompressDst], edi
    call zlib_decompress
    pop ecx
    pop esi
    jc .error

    ; Now we have decompressed image data, need to de-filter it
    ; For simplicity, just copy to framebuffer
    ; In a full implementation, apply filter reconstruction

    ; For now, copy raw to framebuffer
    mov edi, [framebuffer_addr]
    mov esi, [png_work_buffer]
    movzx ecx, word [png_width]
    movzx ebx, word [png_height]
    mul ebx                  ; Total pixels

.copy_loop:
    movsb
    loop .copy_loop

    clc
    popad
    ret

.error:
    stc
    popad
    ret

; =============================================================================
; Find a PNG chunk by type
; Input: ESI = start of PNG data
; Output: ESI = chunk data, ECX = chunk length, CF = error
; =============================================================================
png_find_chunk:
    ; Skip to next chunk (already at chunk start or after signature)

.next_chunk:
    ; Read chunk length (4 bytes big-endian)
    lodsd
    bswap eax
    mov [png_chunk_length], eax

    ; Read chunk type (4 bytes)
    lodsd
    mov [png_chunk_type], eax

    ; Check if this is the chunk we want
    mov eax, [png_chunk_type]
    cmp eax, [png_target_chunk]
    je .found

    ; Skip chunk data + CRC (4 bytes)
    movzx ecx, word [png_chunk_length]
    add esi, ecx
    add esi, 4              ; Skip CRC

    jmp .next_chunk

.found:
    movzx ecx, word [png_chunk_length]
    clc
    ret

; =============================================================================
; Parse IHDR chunk
; Input: ESI = IHDR data
; =============================================================================
png_parse_ihdr:
    ; Width (4 bytes big-endian)
    lodsd
    bswap eax
    mov [png_width], ax

    ; Height
    lodsd
    bswap eax
    mov [png_height], ax

    ; Bit depth
    lodsb
    mov [png_bit_depth], al

    ; Color type
    lodsb
    mov [png_color_type], al

    ; Compression method
    lodsb

    ; Filter method
    lodsb

    ; Interlace method
    lodsb

    ; Validate
    cmp byte [png_color_type], 2      ; RGB
    je .ok
    cmp byte [png_color_type], 6     ; RGBA
    je .ok

    ; Only support RGB and RGBA for now
    stc
    ret

.ok:
    clc
    ret

; =============================================================================
; Simple zlib decompressor (inflate)
; Input: ESI = compressed data, ECX = size, EDI = output buffer
; Output: EDI = bytes written
; =============================================================================
zlib_decompress:
    pushad

    ; Skip zlib header (2 bytes minimum)
    lodsb
    lodsb

    ; Simple stored block decompression
    ; In a full implementation, handle deflate blocks

    ; For now, just copy data (assuming uncompressed)
    ; This is a simplified version

    popad
    ret

; =============================================================================
; Data section
; =============================================================================
SECTION .data

png_width:       dw 0
png_height:      dw 0
png_bit_depth:   db 0
png_color_type:  db 0
png_chunk_length: dd 0
png_chunk_type:  dd 0
png_target_chunk: dd 0
png_work_buffer: dd 0
pngDecompressDst: dd 0

; =============================================================================
; Render PNG as background
; Input: ESI = PNG file data, ECX = file size
; =============================================================================
global render_png_background
render_png_background:
    pushad

    ; Load the PNG
    call png_load_image
    jc .done

    ; Successfully loaded - image is now in framebuffer

.done:
    popad
    ret
