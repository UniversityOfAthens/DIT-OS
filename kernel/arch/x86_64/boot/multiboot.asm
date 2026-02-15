; =============================================================================
; DIT-OS Kernel - Multiboot Header (32-bit)
; =============================================================================

section .multiboot
align 4

multiboot_header:
    dd 0x1BADB002                ; Multiboot magic
    dd 0x00010003                ; Flags (memory info + video)
    dd -(0x1BADB002 + 0x00010003) ; Checksum
    
    ; Additional info
    dd 0                         ; Header address
    dd 0                         ; Load address
    dd 0                         ; Load end address
    dd 0                         ; BSS end address
    dd 0                         ; Entry address
    
    ; Video mode
    dd 1                         ; 1 = linear graphics
    dd 800                       ; Width
    dd 600                       ; Height
    dd 32                        ; Depth
