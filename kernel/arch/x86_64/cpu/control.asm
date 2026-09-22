; =============================================================================
; DIT-OS Kernel - Control registers (CR0, CR3, CR4)
; =============================================================================

BITS 32

section .text

global read_cr0_impl
global write_cr0_impl
global read_cr2_impl
global read_cr3_impl
global write_cr3_impl
global read_cr4_impl
global write_cr4_impl

read_cr0_impl:
    mov eax, cr0
    ret

write_cr0_impl:
    mov eax, [esp + 4]
    mov cr0, eax
    ret

read_cr2_impl:
    mov eax, cr2
    ret

read_cr3_impl:
    mov eax, cr3
    ret

write_cr3_impl:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

read_cr4_impl:
    mov eax, cr4
    ret

write_cr4_impl:
    mov eax, [esp + 4]
    mov cr4, eax
    ret
