; =============================================================================
; A20 Line Enable
; =============================================================================
GLOBAL enable_a20
enable_a20:
    ; Fast A20 method
    in al, 0x92
    or al, 2
    out 0x92, al
    ret
GLOBAL disable_a20
disable_a20:
    in al, 0x92
    and al, 0xFD
    out 0x92, al
    ret
