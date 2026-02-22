; =============================================================================
; DIT-OS Kernel - Model Specific Registers (RDMSR / WRMSR)
; =============================================================================

BITS 32

section .text

; uint64_t rdmsr_impl(uint32_t msr);  returns in edx:eax
global rdmsr_impl
rdmsr_impl:
    mov ecx, [esp + 4]
    rdmsr
    ret

; void wrmsr_impl(uint32_t msr, uint64_t value);
global wrmsr_impl
wrmsr_impl:
    mov ecx, [esp + 4]
    mov eax, [esp + 8]
    mov edx, [esp + 12]
    wrmsr
    ret
