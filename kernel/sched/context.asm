; =============================================================================
; DIT-OS Kernel - Context switch
; switch_to(prev, next): save the current context on its stack and in prev,
; load next's stack and resume it.
;
; Stack frame of a switched-out task (low to high address):
;   edi, esi, ebp, ebx, eflags, return eip
; EFLAGS is saved so a task that blocked with interrupts disabled does not
; hand IF=0 to the task it switches to. task_init_stack() in task.cpp builds
; the same frame for new tasks.
; Context struct layout: edi, esi, ebp, ebx, esp, eip (6 dwords)
; =============================================================================

BITS 32

section .text

global switch_to
switch_to:
    ; [esp+4] = prev, [esp+8] = next
    mov eax, [esp+4]   ; prev
    mov edx, [esp+8]   ; next

    pushfd
    push ebx
    push ebp
    push esi
    push edi

    ; Save current esp (after the pushes) to prev->ctx.esp
    mov [eax+16], esp
    ; Save return address (eip when we'll resume) to prev->ctx.eip
    mov ecx, [esp+20]
    mov [eax+20], ecx
    ; Save the callee-saved regs to prev->ctx (for debugging)
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
    popfd
    ret                 ; pops next's saved eip and jumps

global user_entry_trap
user_entry_trap:
    mov esp, edi
    iret
