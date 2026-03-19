; =============================================================================
; DIT-OS Kernel - Context switch (save/restore callee-saved regs + esp/eip)
; switch_to(prev, next): save current context to prev, load next's context, jump to next.
; Context layout: edi, esi, ebp, ebx, esp, eip (6 dwords)
; =============================================================================

BITS 32

section .text

global switch_to
switch_to:
    ; [esp+4] = prev, [esp+8] = next
    mov eax, [esp+4]   ; prev
    mov edx, [esp+8]   ; next

    ; Save callee-saved regs (we're about to push them so we can save to prev)
    push ebx
    push ebp
    push esi
    push edi

    ; Save current esp (after the 4 pushes) to prev->ctx.esp
    mov [eax+16], esp
    ; Save return address (eip when we'll resume) to prev->ctx.eip
    mov ecx, [esp+16]
    mov [eax+20], ecx
    ; Save the 4 callee-saved regs we just pushed to prev->ctx
    mov [eax], edi
    mov [eax+4], esi
    mov [eax+8], ebp
    mov [eax+12], ebx

    ; Restore next's context
    mov esp, [edx+16]   ; next->ctx.esp
    pop edi
    pop esi
    pop ebp
    pop ebx
    ret                 ; pops next->ctx.eip (from next's stack) and jumps
