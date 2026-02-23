; =============================================================================
; DIT-OS Loader - Parse ELF kernel, set Multiboot, jump to kernel
; Loaded by stage2 to 0x1000:0 = physical 0x10000.
; Kernel ELF raw bytes are at physical 0x20000 (loaded by stage2).
; =============================================================================

ELF_MAGIC    equ 0x464C457F
PT_LOAD      equ 1
MB_MAGIC     equ 0x2BADB002
KERNEL_BUF   equ 0x20000
LOAD_BASE    equ 0x10000

COM1         equ 0x3F8

    [bits 16]
    [org 0]

; ---------- 16-bit entry (CS=0x1000, IP=0) ----------
start16:
    cli
    mov ax, 0x1000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xE000

    ; Serial: print "L" so we know the loader started
    mov dx, COM1 + 5
.ser_wait1:
    in al, dx
    test al, 0x20
    jz .ser_wait1
    mov dx, COM1
    mov al, 'L'
    out dx, al

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Patch GDT descriptor with physical address of GDT
    mov eax, LOAD_BASE
    add eax, gdt
    mov [gdt_desc + 2], eax
    lgdt [gdt_desc]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; 32-bit far jump (0x66 prefix gives 32-bit offset, works even in 16-bit mode)
    jmp dword 0x08:(LOAD_BASE + pm_entry)

; ---------- GDT: flat base=0 ----------
align 8
gdt:
    dq 0                             ; null
    dw 0xFFFF, 0x0000                ; 0x08 code32 base=0
    db 0x00, 0x9A, 0xCF, 0x00
    dw 0xFFFF, 0x0000                ; 0x10 data32 base=0
    db 0x00, 0x92, 0xCF, 0x00
gdt_end:
gdt_desc:
    dw gdt_end - gdt - 1
    dd 0                             ; patched at runtime

; ---------- 32-bit protected mode ----------
    [bits 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9F000

    ; Serial: print "O" (loader OK in pmode)
    mov dx, COM1 + 5
.ser_wait2:
    in al, dx
    test al, 0x20
    jz .ser_wait2
    mov dx, COM1
    mov al, 'O'
    out dx, al

    ; Verify ELF magic at KERNEL_BUF (physical 0x20000)
    mov eax, [KERNEL_BUF]
    cmp eax, ELF_MAGIC
    jne .bad_elf

    ; Serial: print "E" (ELF found)
    mov dx, COM1 + 5
.ser_wait3:
    in al, dx
    test al, 0x20
    jz .ser_wait3
    mov dx, COM1
    mov al, 'E'
    out dx, al

    ; Save e_entry
    mov ecx, [KERNEL_BUF + 0x18]
    mov [LOAD_BASE + kernel_entry], ecx

    ; Walk program headers
    mov eax, [KERNEL_BUF + 0x1C]     ; e_phoff
    add eax, KERNEL_BUF
    movzx ecx, word [KERNEL_BUF + 0x2C]  ; e_phnum
    movzx edx, word [KERNEL_BUF + 0x2A]  ; e_phentsize

.ph_loop:
    test ecx, ecx
    jz .ph_done
    cmp dword [eax], PT_LOAD
    jne .ph_next

    ; Save loop state
    mov [LOAD_BASE + save_eax], eax
    mov [LOAD_BASE + save_ecx], ecx
    mov [LOAD_BASE + save_edx], edx

    ; Copy p_filesz bytes from (KERNEL_BUF + p_offset) to p_vaddr
    mov esi, [eax + 4]               ; p_offset
    add esi, KERNEL_BUF
    mov edi, [eax + 8]               ; p_vaddr
    mov ecx, [eax + 16]              ; p_filesz
    cld
    rep movsb
    ; Zero (p_memsz - p_filesz) bytes for BSS
    mov eax, [LOAD_BASE + save_eax]
    mov ecx, [eax + 20]              ; p_memsz
    sub ecx, [eax + 16]              ; - p_filesz
    jbe .no_bss
    xor al, al
    rep stosb
.no_bss:
    ; Restore loop state
    mov eax, [LOAD_BASE + save_eax]
    mov ecx, [LOAD_BASE + save_ecx]
    mov edx, [LOAD_BASE + save_edx]

.ph_next:
    add eax, edx
    dec ecx
    jmp .ph_loop

.ph_done:
    ; Serial: print "K\r\n" (jumping to kernel)
    mov dx, COM1 + 5
.ser_wait4:
    in al, dx
    test al, 0x20
    jz .ser_wait4
    mov dx, COM1
    mov al, 'K'
    out dx, al

    ; Multiboot: EAX = magic, EBX = &mb_info (physical)
    mov ebx, LOAD_BASE + mb_info
    mov eax, MB_MAGIC
    ; Jump to kernel entry point
    jmp [LOAD_BASE + kernel_entry]

.bad_elf:
    ; Serial: print "ELF?\r\n"
    mov esi, LOAD_BASE + msg_bad
    mov dx, COM1
.bad_loop:
    mov al, [esi]
    test al, al
    jz .hang
    push edx
    mov dx, COM1 + 5
.bad_wait:
    in al, dx
    test al, 0x20
    jz .bad_wait
    pop edx
    mov al, [esi]
    out dx, al
    inc esi
    jmp .bad_loop
.hang:
    hlt
    jmp .hang

; ---------- Data ----------
kernel_entry: dd 0
save_eax: dd 0
save_ecx: dd 0
save_edx: dd 0
msg_bad: db "ELF?", 13, 10, 0

align 4
mb_info:
    dd 0    ; flags
    dd 640  ; mem_lower (KB)
    dd 0    ; mem_upper (KB) - placeholder
    times 52 db 0
