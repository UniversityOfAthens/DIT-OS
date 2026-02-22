; =============================================================================
; E820 Memory Map - INT 15h EAX=E820
; =============================================================================

BITS 16

GLOBAL memory_e820

; void memory_e820(void *buffer, int max_entries);
; Buffer: array of 20-byte entries. Returns count in AX.
; C signature: int memory_e820(struct e820_entry *entries, int max);

SECTION .text

memory_e820:
    push bp
    mov bp, sp
    push di
    push si
    push bx
    push cx

    mov di, [bp + 4]   ; buffer (offset)
    mov bx, [bp + 6]   ; segment for buffer (if far)
    mov cx, [bp + 8]   ; max_entries

    xor si, si         ; count

.e820_loop:
    mov eax, 0xE820
    mov edx, 0x534D4150   ; "SMAP"
    mov ecx, 20
    int 0x15
    jc .e820_done
    cmp eax, 0x534D4150
    jne .e820_done

    inc si
    add di, 20
    cmp si, cx
    jge .e820_done

    test ebx, ebx
    jnz .e820_loop

.e820_done:
    mov ax, si
    pop cx
    pop bx
    pop si
    pop di
    pop bp
    ret
