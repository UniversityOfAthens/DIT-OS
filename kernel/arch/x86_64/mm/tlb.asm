; =============================================================================
; DIT-OS Kernel - TLB management
; =============================================================================

BITS 32

section .text

; void tlb_invalidate_impl(uint32_t vaddr);
global tlb_invalidate_impl
tlb_invalidate_impl:
    mov eax, [esp + 4]
    invlpg [eax]
    ret

; void tlb_flush_impl(void);
global tlb_flush_impl
tlb_flush_impl:
    mov eax, cr3
    mov cr3, eax
    ret
