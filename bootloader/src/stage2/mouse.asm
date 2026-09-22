; =============================================================================
; DIT-OS Stage 2 - PS/2 Mouse Support
; =============================================================================

; Mouse constants
MOUSE_PORT        equ 0x60
MOUSE_STATUS      equ 0x64
MOUSE_CMD         equ 0x64

MOUSE_ENABBLE     equ 0xF4
MOUSE_DISABLE     equ 0xF5
MOUSE_GET_STATUS  equ 0xE9
MOUSE_SET_SAMPLE  equ 0xF3

PS2_CMD_WRITEAUX  equ 0xD4
PS2_CMD_ENABLE    equ 0xA8
PS2_CMD_DISABLE   equ 0xA7

; Mouse packet flags
MOUSE_BUTTON_LEFT   equ 0x01
MOUSE_BUTTON_RIGHT  equ 0x02
MOUSE_BUTTON_MIDDLE equ 0x04
MOUSE_X_SIGN       equ 0x10
MOUSE_Y_SIGN       equ 0x20
MOUSE_X_OVERFLOW   equ 0x40
MOUSE_Y_OVERFLOW   equ 0x80

GLOBAL mouse_init
GLOBAL mouse_available
GLOBAL mouse_x
GLOBAL mouse_y
GLOBAL mouse_buttons
GLOBAL mouse_get_position
GLOBAL mouse_enable
GLOBAL mouse_disable

; Mouse state
mouse_available: db 0
mouse_x: dw 320
mouse_y: dw 240
mouse_buttons: db 0
mouse_packet_ready: db 0
mouse_packet_byte: db 0

mouse_x_sign: db 0
mouse_y_sign: db 0
mouse_x_overflow: db 0
mouse_y_overflow: db 0

screen_center_x: dw 320
screen_center_y: dw 240

; =============================================================================
; Initialize PS/2 mouse
; =============================================================================
mouse_init:
    pusha

    mov si, .msg_start
    call print_string

    ; Enable auxiliary PS/2 device
    mov al, PS2_CMD_ENABLE
    out MOUSE_CMD, al

    ; Send mouse enable command
    mov al, MOUSE_ENABBLE
    call mouse_send_command

    ; Set sample rate (40 reports/sec)
    mov al, 0xC8
    call mouse_send_command

    ; Set resolution (8 counts per mm)
    mov al, 0x03
    call mouse_send_command

    ; Mark as available
    mov byte [mouse_available], 1

    mov si, .msg_ok
    call print_string

    popa
    ret

.msg_start: db "Initializing mouse...", 13, 10, 0
.msg_ok:    db "Mouse initialized", 13, 10, 0

; =============================================================================
; Send command to mouse
; =============================================================================
mouse_send_command:
    pusha

    ; Wait for input buffer to be empty
.wait1:
    in al, MOUSE_STATUS
    and al, 0x02
    jnz .wait1

    ; Send command
    mov al, PS2_CMD_WRITEAUX
    out MOUSE_CMD, al

.wait2:
    in al, MOUSE_STATUS
    and al, 0x02
    jnz .wait2

    ; Send data
    popa
    out MOUSE_PORT, al
    pusha

    ; Wait for data
.wait3:
    in al, MOUSE_STATUS
    and al, 0x01
    jz .wait3

    ; Read response (ACK)
    in al, MOUSE_PORT

    popa
    ret

; =============================================================================
; Enable mouse interrupt
; =============================================================================
mouse_enable:
    pusha
    cmp byte [mouse_available], 0
    je .done

    mov al, MOUSE_ENABBLE
    call mouse_send_command

.done:
    popa
    ret

; =============================================================================
; Disable mouse
; =============================================================================
mouse_disable:
    pusha
    cmp byte [mouse_available], 0
    je .done

    mov al, MOUSE_DISABLE
    call mouse_send_command

.done:
    popa
    ret

; =============================================================================
; Read mouse packet (call from interrupt handler)
; Input: AL = mouse data
; =============================================================================
mouse_handle_packet:
    pusha

    movzx ebx, byte [mouse_packet_byte]

    ; First byte
    cmp bl, 0
    jne .byte1

    ; Check for overflow
    test al, MOUSE_X_OVERFLOW
    jz .no_xovf
    mov byte [mouse_x_overflow], 1
.no_xovf:
    test al, MOUSE_Y_OVERFLOW
    jz .no_yovf
    mov byte [mouse_y_overflow], 1
.no_yovf:

    ; Save flags
    mov [mouse_x_sign], al
    and al, MOUSE_X_SIGN
    mov [mouse_x_sign], al

    movzx eax, byte [mouse_x_sign]
    and al, MOUSE_Y_SIGN
    mov [mouse_y_sign], al

    ; Buttons
    and bl, MOUSE_BUTTON_LEFT | MOUSE_BUTTON_RIGHT | MOUSE_BUTTON_MIDDLE
    mov [mouse_buttons], bl

    inc byte [mouse_packet_byte]
    jmp .done

.byte1:
    ; Second byte - X movement
    movzx eax, al
    test byte [mouse_x_sign], MOUSE_X_SIGN
    jz .x_positive
    or eax, 0xFFFFFF00
.x_positive:

    ; Add to position (scaled)
    shl eax, 1
    add [mouse_x], ax

    inc byte [mouse_packet_byte]
    jmp .done

.byte2:
    ; Third byte - Y movement
    movzx eax, al
    test byte [mouse_y_sign], MOUSE_Y_SIGN
    jz .y_positive
    or eax, 0xFFFFFF00
.y_positive:

    ; Subtract from position (Y is inverted on screen)
    shl eax, 1
    sub [mouse_y], ax

    ; Reset packet counter
    mov byte [mouse_packet_byte], 0

    ; Clamp to screen bounds
    cmp word [mouse_x], 0
    jl .clamp_x
    cmp word [mouse_x], 639
    jg .clamp_x
    jmp .x_ok
.clamp_x:
    mov word [mouse_x], 320

.x_ok:
    cmp word [mouse_y], 0
    jl .clamp_y
    cmp word [mouse_y], 479
    jg .clamp_y
    jmp .y_ok
.clamp_y:
    mov word [mouse_y], 240

.y_ok:
    ; Signal packet ready
    mov byte [mouse_packet_ready], 1

.done:
    popa
    ret

; =============================================================================
; Get mouse position
; Output: AX = x, BX = y, CL = buttons
; =============================================================================
mouse_get_position:
    pusha

    mov ax, [mouse_x]
    mov bx, [mouse_y]
    mov cl, [mouse_buttons]

    mov byte [mouse_packet_ready], 0

    popa
    ret

; =============================================================================
; Poll mouse (for non-interrupt mode)
; =============================================================================
mouse_poll:
    pusha

    cmp byte [mouse_available], 0
    je .done

    ; Check if there's data
    in al, MOUSE_STATUS
    and al, 0x01
    jz .done

    ; Read data
    in al, MOUSE_PORT

    ; Check for mouse data (bit 5 of status)
    in ah, MOUSE_STATUS
    test ah, 0x20
    jnz .is_mouse_data

    ; Keyboard data, ignore
    jmp .done

.is_mouse_data:
    call mouse_handle_packet

.done:
    popa
    ret
