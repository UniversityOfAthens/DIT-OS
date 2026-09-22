; =============================================================================
; DIT-OS Stage 2 - PF2 Font Loader
; PC Screen Font format used by GRUB
; =============================================================================

; External symbols
extern plot_pixel
extern framebuffer_addr
extern font_data

; PF2 constants
PF2_MAGIC: db 0x25, 0x66, 0x6F, 0x6E  ; "%fon"

; PF2 flags
PF2_NO_WRITE:   equ 0x01
PF2_COMPRESSED: equ 0x02
PF2_BROKEN:     equ 0x04
PF2_VARWIDTH:   equ 0x08

; =============================================================================
; Load PF2 font
; Input: ESI = font file data
; Output: EAX = font structure pointer, CF = error
; =============================================================================
global pf2_load_font
pf2_load_font:
    pushad

    ; Check PF2 magic
    lodsd
    cmp eax, 0x6E6F6625            ; "%fon" in little-endian
    jne .error

    ; Skip font version (4 bytes)
    lodsd

    ; Read font structure size
    lodsd
    mov [pf2_struct_size], eax

    ; Read flags
    lodsd
    mov [pf2_flags], eax

    ; Read number of glyphs
    lodsd
    mov [pf2_glyph_count], eax

    ; Read font height
    lodsd
    mov [pf2_font_height], eax

    ; Read font width (for fixed-width)
    lodsd
    mov [pf2_font_width], eax

    ; Skip reserved bytes
    add esi, 12

    ; Skip font name (max 258 bytes)
    xor ax, ax
    lodsb
    add esi, eax

    ; Save glyph table offset
    mov [pf2_glyph_table], esi

    ; Allocate font structure
    mov eax, pf2_font_data
    mov [current_font], eax

    ; Copy font info
    mov edi, eax
    mov eax, [pf2_font_height]
    stosd
    mov eax, [pf2_font_width]
    stosd
    mov eax, [pf2_glyph_count]
    stosd
    mov eax, [pf2_glyph_table]
    stosd

    clc
    popad
    mov eax, [current_font]
    ret

.error:
    stc
    popad
    ret

; =============================================================================
; Get glyph bitmap
; Input: EAX = character code
; Output: ESI = glyph data, ECX = glyph size, CF = error
; =============================================================================
global pf2_get_glyph
pf2_get_glyph:
    pushad

    ; Check if font is loaded
    mov esi, [current_font]
    test esi, esi
    jz .error

    ; Get glyph count
    mov eax, [esi + 8]
    mov [pf2_max_char], eax

    ; Check if character is in range
    movzx ebx, byte [character_code]
    cmp ebx, [pf2_max_char]
    jae .not_found

    ; Get glyph table pointer
    mov esi, [esi + 12]

    ; Each glyph has:
    ; - character (1 byte)
    ; - width (1 byte)
    ; - height (1 byte)
    ; - x offset (1 byte)
    ; - y offset (1 byte)
    ; - device width (1 byte)
    ; - bitmap data (variable)

    ; Calculate glyph offset
    ; Glyph header = 8 bytes, then bitmap
    movzx eax, bl
    mov ecx, 8                      ; Glyph header size
    mul ecx
    add esi, eax

    ; Check character match
    lodsb
    cmp al, [character_code]
    jne .not_found

    ; Get glyph dimensions
    lodsb                           ; Width
    mov [glyph_width], al
    lodsb                           ; Height
    mov [glyph_height], al

    ; Skip x, y offset and device width (3 bytes)
    add esi, 3

    ; Calculate bitmap size
    movzx eax, byte [glyph_width]
    movzx ebx, byte [glyph_height]
    mul ebx
    mov [glyph_size], eax

    ; Return pointer to bitmap
    mov esi, [current_font]
    add esi, PF2_FONT_DATA_SIZE

    clc
    popad
    ret

.not_found:
.error:
    stc
    popad
    ret

; =============================================================================
; Draw character using PF2 font
; Input: AL = character, ECX = x, EDX = y, EBX = color
; =============================================================================
global pf2_draw_char
pf2_draw_char:
    pushad

    mov [character_code], al

    ; Get glyph
    call pf2_get_glyph
    jc .done

    ; Use standard font if PF2 not loaded
    test esi, esi
    jz .use_default

    ; Draw glyph from PF2 font
    jmp .draw

.use_default:
    ; Fall back to built-in font
    mov esi, font_data

.draw:
    ; Draw character (similar to existing draw_char)
    ; This is simplified - full implementation would handle
    ; variable-width PF2 glyphs

    popad
    ret

.done:
    popad
    ret

; =============================================================================
; Data section
; =============================================================================
SECTION .data

pf2_struct_size:   dd 0
pf2_flags:         dd 0
pf2_glyph_count:  dd 0
pf2_font_height:   dd 0
pf2_font_width:    dd 0
pf2_glyph_table:   dd 0
pf2_max_char:      dd 0

glyph_width:      db 0
glyph_height:     db 0
glyph_size:       dd 0

character_code:   db 0

current_font:     dd 0

; Font structure
pf2_font_data:
pf2_height:       dd 0
pf2_width:        dd 0
pf2_count:        dd 0
pf2_glyph_ptr:    dd 0

PF2_FONT_DATA_SIZE equ 16

; Buffer for loaded PF2 font
times 4096 db 0                    ; Font data buffer
