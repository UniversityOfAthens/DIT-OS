; =============================================================================
; DIT-OS Stage 2 - FAT32 Filesystem Driver
; For reading files from FAT32 partitions
; =============================================================================

; External functions
extern read_sectors

; FAT32 constants
FAT32_SIGNATURE         equ 0xAA55
FAT32_EOC               equ 0x0FFFFFF8

; Boot sector offsets
BS_OEMName:             equ 3
BS_BytesPerSec:         equ 11
BS_SecPerClus:          equ 13
BS_RsvdSecCnt:          equ 14
BS_NumFATs:             equ 16
BS_TotSec16:            equ 19
BS_Media:               equ 21
BS_FATSz16:             equ 22
BS_SecPerTrk:           equ 24
BS_NumHeads:            equ 26
BS_HiddSec:             equ 28
BS_TotSec32:            equ 32

; FAT32 extended boot sector
BS_FATSz32:             equ 36
BS_ExtFlags:            equ 40
BS_FSVer:               equ 42
BS_RootClus:            equ 44
BS_FSInfo:              equ 48
BS_BkBootSec:           equ 50
BS_DriveNumber:         equ 64
BS_VolumeID:            equ 67
BS_VolumeLabel:         equ 71
BS_FileSystemType:      equ 82

; =============================================================================
; Initialize FAT32 filesystem
; Input: ESI = partition start LBA
; =============================================================================
global fat32_init
fat32_init:
    pushad

    ; Read boot sector
    mov eax, [partition_start]
    mov ebx, fat32_buffer
    mov cx, 1
    call read_sectors

    ; Verify signature
    movzx eax, word [ebx + 510]
    cmp ax, FAT32_SIGNATURE
    jne .error

    ; Get filesystem parameters
    movzx eax, word [ebx + BS_BytesPerSec]
    mov [fat_bytes_per_sector], ax

    mov al, [ebx + BS_SecPerClus]
    mov [fat_sectors_per_cluster], al

    movzx eax, word [ebx + BS_RsvdSecCnt]
    mov [fat_reserved_sectors], ax

    mov al, [ebx + BS_NumFATs]
    mov [fat_num_fats], al

    ; Get FAT size (32-bit)
    mov eax, [ebx + BS_FATSz32]
    mov [fat_fat_size], eax

    ; Get root cluster
    mov eax, [ebx + BS_RootClus]
    mov [fat_root_cluster], eax

    ; Calculate data sector start
    movzx ebx, word [fat_reserved_sectors]
    movzx ecx, byte [fat_num_fats]
    mov eax, [fat_fat_size]
    mul ecx
    add ebx, eax                    ; EBX = first data sector
    mov [fat_data_start], ebx

    ; Mark as initialized
    mov byte [fat32_initialized], 1

    clc
    popad
    ret

.error:
    stc
    popad
    ret

; =============================================================================
; Open a file
; Input: ESI = null-terminated filename
; Output: EAX = file handle or 0 if not found
; =============================================================================
global fat32_open_file
fat32_open_file:
    pushad

    ; Mark file not found
    xor eax, eax
    mov [file_handle], eax

    ; Save filename
    mov edi, filename_buffer
    mov ecx, 256
    rep movsb

    ; Start searching from root cluster
    mov eax, [fat_root_cluster]
    mov [current_cluster], eax

.search_loop:
    ; Read current cluster
    mov eax, [current_cluster]
    call fat32_read_cluster

    ; Search for filename in directory entries
    mov esi, fat32_buffer
    mov cx, 0

.dir_entry_loop:
    ; Check if entry is empty or deleted
    cmp byte [esi], 0
    je .not_found
    cmp byte [esi], 0xE5
    je .next_entry

    ; Check if this is a file (not directory)
    mov al, [esi + 11]
    and al, 0x10
    jnz .next_entry

    ; Compare filename
    push esi
    push cx
    mov edi, filename_buffer
    mov cx, 11                     ; 8.3 format
    rep cmpsb
    pop cx
    pop esi
    je .found

.next_entry:
    add esi, 32                    ; Next directory entry
    inc cx
    cmp cx, [entries_per_cluster]
    jb .dir_entry_loop

    ; Move to next cluster in chain
    call fat32_get_next_cluster
    jc .not_found
    mov [current_cluster], eax
    jmp .search_loop

.not_found:
    stc
    popad
    ret

.found:
    ; Get file info
    mov eax, [esi + 26]            ; First cluster (low)
    mov ebx, [esi + 20]            ; First cluster (high)
    shl ebx, 16
    or eax, ebx
    mov [file_cluster], eax

    mov eax, [esi + 28]            ; File size
    mov [file_size], eax

    ; Create file handle
    mov eax, 1
    mov [file_handle], eax

    clc
    popad
    mov eax, [file_handle]
    ret

; =============================================================================
; Read from open file
; Input: EAX = bytes to read
; Output: EDI = buffer, ECX = bytes read
; =============================================================================
global fat32_read_file
fat32_read_file:
    pushad

    test eax, eax
    jz .done

    ; Limit to file size
    cmp eax, [file_size]
    jbe .ok_size
    mov eax, [file_size]

.ok_size:
    mov [bytes_to_read], eax

    ; Get starting cluster
    mov eax, [file_cluster]
    mov [current_cluster], eax
    mov [current_offset], eax

.read_loop:
    ; Check if done
    cmp dword [bytes_to_read], 0
    je .done

    ; Read cluster
    mov eax, [current_cluster]
    call fat32_read_cluster

    ; Copy data
    mov esi, fat32_buffer
    mov edi, [read_buffer]
    movzx ecx, byte [fat_sectors_per_cluster]
    movzx ebx, word [fat_bytes_per_sector]
    mul ebx
    mov ecx, eax

    cmp ecx, [bytes_to_read]
    jbe .copy_all

    mov ecx, [bytes_to_read]

.copy_all:
    rep movsb

    ; Update counters
    sub [bytes_to_read], ecx
    add [read_buffer], ecx

    ; Get next cluster
    call fat32_get_next_cluster
    jc .done

    mov [current_cluster], eax
    jmp .read_loop

.done:
    mov eax, [read_buffer]
    sub eax, [file_buffer]
    mov [bytes_read], eax

    popad
    mov ecx, [bytes_read]
    ret

; =============================================================================
; Get next cluster in FAT chain
; Input: EAX = current cluster
; Output: EAX = next cluster, CF = end of chain
; =============================================================================
fat32_get_next_cluster:
    push ebx

    ; Calculate FAT entry offset
    mov ebx, eax
    shl ebx, 2                      ; *4 for 32-bit entries
    add ebx, [fat_table_offset]

    ; Read FAT entry
    mov eax, [ebx]
    and eax, 0x0FFFFFFF            ; Mask out reserved bits

    ; Check for end of chain
    cmp eax, FAT32_EOC
    jae .eoc

    clc
    pop ebx
    ret

.eoc:
    stc
    pop ebx
    ret

; =============================================================================
; Read a cluster
; Input: EAX = cluster number
; =============================================================================
fat32_read_cluster:
    pushad

    ; Convert cluster to LBA
    ; LBA = (cluster - 2) * sectors_per_cluster + data_start

    sub eax, 2
    movzx ebx, byte [fat_sectors_per_cluster]
    mul ebx
    add eax, [fat_data_start]
    add eax, [partition_start]

    ; Read sectors
    mov ebx, fat32_buffer
    movzx cx, byte [fat_sectors_per_cluster]
    call read_sectors

    popad
    ret

; =============================================================================
; Data section
; =============================================================================
SECTION .data

partition_start:     dd 0               ; LBA of partition start
fat32_initialized:   db 0

; Filesystem parameters
fat_bytes_per_sector: dw 512
fat_sectors_per_cluster: db 0
fat_reserved_sectors: dw 0
fat_num_fats:        db 0
fat_fat_size:        dd 0
fat_root_cluster:     dd 0
fat_data_start:      dd 0
fat_table_offset:    dd 0

entries_per_cluster: dw 0

; File info
current_cluster:     dd 0
current_offset:      dd 0
file_cluster:       dd 0
file_size:          dd 0
file_handle:        dd 0

filename_buffer:    times 256 db 0
fat32_buffer:       times 65536 db 0    ; 64KB buffer
file_buffer:        times 65536 db 0    ; 64KB file buffer
read_buffer:        dd 0
bytes_to_read:      dd 0
bytes_read:         dd 0
