; =============================================================================
; Menu Functions (ASM utilities)
; =============================================================================
GLOBAL clear_screen
clear_screen:
    pusha
    mov ah, 0x07
    mov al, 0
    xor cx, cx
    mov dh, 0x18
    mov dl, 0x4F
    int 0x10
    popa
    ret
GLOBAL set_cursor
set_cursor:
    ; DH = row, DL = column
    mov ah, 0x02
    mov bh, 0
    int 0x10
    ret
GLOBAL get_cursor_pos
get_cursor_pos:
    mov ah, 0x03
    mov bh, 0
    int 0x10
    ; DH = row, DL = column
    ret
