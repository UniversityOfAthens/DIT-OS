BITS 32

global _start

extern main
extern _exit
extern __bss_start
extern __bss_end

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
