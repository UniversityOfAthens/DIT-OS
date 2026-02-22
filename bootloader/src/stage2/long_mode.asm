; =============================================================================
; Long Mode (64-bit) Switch
; Call from 32-bit protected mode. Sets up PAE paging and switches to long mode.
; =============================================================================

BITS 32

GLOBAL enter_long_mode
EXTERN long_mode_entry

; PML4 at 0x1000, PDPT at 0x2000, PD at 0x3000 (identity map first 2MB)
PML4    equ 0x1000
PDPT    equ 0x2000
PD      equ 0x3000

SECTION .text

enter_long_mode:
    ; Identity map first 2MB via PML4 -> PDPT -> PD -> 2MB page
    mov edi, PML4
    xor eax, eax
    mov ecx, 0x1000 / 4
    rep stosd

    mov dword [PML4], (PDPT | 3)
    mov dword [PDPT], (PD | 3)
    mov dword [PD], (0 | 0x83)    ; 2MB page, present, rw

    ; Load PML4 into CR3
    mov eax, PML4
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Set long mode in EFER (EFER MSR = 0xC0000080)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load 64-bit GDT and jump to long mode
    lgdt [long_mode_gdtr]
    jmp 0x08:.lm64

BITS 64
.lm64:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000

    call long_mode_entry
    cli
.hang:
    hlt
    jmp .hang

BITS 32
align 8
long_mode_gdt:
    dq 0
    ; 64-bit code
    dw 0xFFFF, 0
    db 0, 0x9A, 0xAF, 0
    ; 64-bit data
    dw 0xFFFF, 0
    db 0, 0x92, 0xAF, 0
long_mode_gdt_end:

long_mode_gdtr:
    dw long_mode_gdt_end - long_mode_gdt - 1
    dq long_mode_gdt
