BITS 32

global _start

extern main
extern _exit
extern __bss_start
extern __bss_end

; Must match SYS_sigreturn in include/syscall.h.
SYS_SIGRETURN equ 20

_start:
    mov ax, 0x2b
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    cld
    rep stosb

    mov eax, [esp]
    lea ebx, [esp + 4]
    push ebx
    push eax
    call main
    add esp, 8

    push eax
    call _exit
    hlt
    jmp $

; Signal handlers return here (the kernel puts this address below the
; handler's argument). Drop the signal number so ESP points at the saved
; frame, then let the kernel restore the interrupted context.
global __dit_sig_restorer
__dit_sig_restorer:
    add esp, 4
    mov eax, SYS_SIGRETURN
    int 0x80
    hlt
    jmp $
