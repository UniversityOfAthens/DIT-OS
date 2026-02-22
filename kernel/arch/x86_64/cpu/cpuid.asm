; =============================================================================
; DIT-OS Kernel - CPUID
; =============================================================================

BITS 32

section .text

; void cpuid_impl(uint32_t eax_in, uint32_t ecx_in, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx);
global cpuid_impl
cpuid_impl:
    push ebx
    push esi
    mov eax, [esp + 12]
    mov ecx, [esp + 16]
    mov esi, [esp + 20]
    cpuid
    mov [esi], eax
    mov [esi + 4], ebx
    mov [esi + 8], ecx
    mov [esi + 12], edx
    pop esi
    pop ebx
    ret
