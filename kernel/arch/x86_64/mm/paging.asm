; =============================================================================
; DIT-OS Kernel - Paging (32-bit): load CR3, enable/disable paging
; =============================================================================

BITS 32

section .text

; void paging_load_impl(uint32_t pd_physical);
global paging_load_impl
paging_load_impl:
    mov eax, [esp + 4]
    mov cr3, eax
    ret

; void paging_enable_impl(void);
global paging_enable_impl
paging_enable_impl:
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax
    ret

; void paging_disable_impl(void);
global paging_disable_impl
paging_disable_impl:
    mov eax, cr0
    and eax, 0x7FFFFFFF
    mov cr0, eax
    ret
