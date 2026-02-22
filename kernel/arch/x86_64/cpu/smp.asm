; =============================================================================
; DIT-OS Kernel - SMP (stub; AP startup would go here)
; =============================================================================

BITS 32

section .text

global smp_ap_entry
smp_ap_entry:
    cli
    hlt
    jmp smp_ap_entry
