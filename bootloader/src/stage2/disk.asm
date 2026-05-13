; =============================================================================
; Disk I/O Operations
; =============================================================================
extern print_string
extern hang

GLOBAL read_sectors
; AH = 0x02 (read) or 0x03 (write)
; AL = number of sectors
; CH = cylinder
; CL = sector
; DH = head
; DL = drive
; ES:BX = buffer
read_sectors:
    pusha
    mov si, 3           ; retry count
    xor bh, bh

.retry:
    mov ah, 0x02
    int 0x13
    jnc .done
    dec si
    jnz .retry

    mov si, .error_msg
    call print_string
    jmp hang

.done:
    popa
    ret
.error_msg: db "Disk I/O Error!", 0
; Extended read (LBA)
GLOBAL read_lba
read_lba:
    pusha
    
    ; Set up DAP (Disk Address Packet)
    mov si, dap
    mov byte [si], 0x10    ; Size of DAP
    mov byte [si+1], 0      ; Reserved
    mov word [si+2], ax     ; Sector count
    mov word [si+4], bx     ; Buffer offset
    mov word [si+6], 0      ; Buffer segment
    mov dword [si+8], edx   ; LBA (low 32)
    mov dword [si+12], 0    ; LBA (high 32)
    
    mov ah, 0x42
    int 0x13
    jc .error
    
    popa
    ret
.error:
    popa
    ret
SECTION .data
dap: times 16 db 0