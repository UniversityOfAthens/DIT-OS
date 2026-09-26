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

; First instruction of a forked child. EDI is the iret frame to enter user mode
; through and ESI points at the parent's user registers (edi, esi, ebp, esp,
; ebx, edx, ecx), saved at the bottom of this task's kernel stack by
; task_create_fork_child(). The child continues just after the fork() syscall,
; so EAX is 0; every other register has to come back from the saved image.
global fork_entry_trap
fork_entry_trap:
    xor eax, eax
    mov esp, edi
    mov ebx, [esi + 16]
    mov edx, [esi + 20]
    mov ebp, [esi + 8]
    mov edi, [esi + 0]
    mov ecx, [esi + 24]
    mov esi, [esi + 4]
    iret
