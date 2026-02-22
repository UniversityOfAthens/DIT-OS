; =============================================================================
; VESA/VGA video modes - real mode int 10h
; =============================================================================

BITS 16

GLOBAL video_set_mode_asm
GLOBAL video_set_cursor_asm

SECTION .text

; void video_set_mode_asm(uint16_t mode);
video_set_mode_asm:
    push bp
    mov bp, sp
    mov ax, [bp + 4]
    int 0x10
    pop bp
    ret

; void video_set_cursor_asm(int x, int y);
video_set_cursor_asm:
    push bp
    mov bp, sp
    mov ah, 0x02
    mov bh, 0
    mov dh, [bp + 6]   ; y
    mov dl, [bp + 4]   ; x
    int 0x10
    pop bp
    ret
