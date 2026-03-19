; DIT-OS Userland - crt0 (minimal)
;
; Intended for standalone userland ELF executables in the future.
; Not currently used by the kernel-run init task.

BITS 32

global _start

extern main
extern _exit

_start:
    ; Typical C ABI user stack layout:
    ;   [esp]     argc
    ;   [esp+4]   argv[0]
    ;
    ; Call main(argc, argv), then _exit(retval).
    mov eax, [esp]        ; argc
    lea ebx, [esp + 4]   ; argv
    push ebx
    push eax
    call main
    add esp, 8

    push eax
    call _exit

    ; Should not return.
    hlt
    jmp $

