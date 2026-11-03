; =============================================================================
; Constants
; =============================================================================
; Memory addresses
ORG_ADDR         equ 0x7C00      ; Stage1 load address
STAGE2_LOAD      equ 0x7E00      ; Stage2 load address (stage1 jumps here)
KERNEL_LOAD      equ 0x100000   ; Kernel load address (1MB)
; Disk
SECTOR_SIZE      equ 512
STAGE2_SECTORS   equ 9           ; Actual: 9 sectors (matches Makefile)
KERNEL_SECTORS   equ 240         ; Actual: ~212 sectors (0.1.0), 240 with headroom
; Video
VIDEO_MEM        equ 0xB8000     ; Text mode video memory
VIDEO_COLS       equ 80
VIDEO_ROWS       equ 25
; Ports
KEYBOARD_DATA    equ 0x60
KEYBOARD_STATUS  equ 0x64
CMOS_PORT        equ 0x70
; Interrupts
INT_DISK         equ 0x13
INT_KEYBOARD     equ 0x16
INT_VIDEO        equ 0x10
; GDT selector bits
SELECTOR_CODE    equ 0x08
SELECTOR_DATA    equ 0x10
; CPU modes
MODE_REAL        equ 0
MODE_PROTECTED   equ 1
MODE_LONG        equ 2
