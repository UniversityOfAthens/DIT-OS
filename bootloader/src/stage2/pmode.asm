; =============================================================================
; Switch to Protected Mode
; =============================================================================
extern gdt_descriptor

GLOBAL switch_to_protected

switch_to_protected:
    ; Disable interrupts
    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Set CR0 bit 0 (PE bit)
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to set CS
    jmp 0x08:protected_mode
protected_mode:
    ; Set up segment registers
    mov ax, 0x10    ; Data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
