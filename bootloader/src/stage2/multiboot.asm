; =============================================================================
; DIT-OS Stage 2 - Multiboot Support
; Compatible with Multiboot Specification
; =============================================================================

; External symbols
extern graphics_available
extern framebuffer_addr

; Multiboot constants
MULTIBOOT_HEADER_MAGIC     equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS     equ 0x00010003  ; Memory info + video mode
MULTIBOOT_HEADER_CHECKSUM  equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

; Multiboot boot info flags
MB_FLAG_MEM         equ 0x01
MB_FLAG_DEVICE      equ 0x02
MB_FLAG_CMDLINE     equ 0x04
MB_FLAG_MODS        equ 0x08
MB_FLAG_AOUT        equ 0x10
MB_FLAG_ELF         equ 0x20
MB_FLAG_MMAP        equ 0x40
MB_FLAG_DRIVES      equ 0x80
MB_FLAG_CONFIG      equ 0x100
MB_FLAG_BTLOADER    equ 0x200
MB_FLAG_APM         equ 0x400
MB_FLAG_VBE         equ 0x800

; Memory map entry types
MMAP_TYPE_AVAILABLE     equ 1
MMAP_TYPE_RESERVED      equ 2
MMAP_TYPE_ACPI_RECLAIM  equ 3
MMAP_TYPE_NVS          equ 4

; =============================================================================
; Multiboot information structure
; =============================================================================
struc MultibootInfo
    .flags           resd 1
    .mem_lower       resd 1
    .mem_upper       resd 1
    .boot_device     resd 1
    .cmdline         resd 1
    .mods_count      resd 1
    .mods_addr       resd 1
    .syms            resb 20         ; AOUT/ELF symbols
    .mmap_length     resd 1
    .mmap_addr       resd 1
    .drives_length   resd 1
    .drives_addr     resd 1
    .config_table    resd 1
    .boot_loader_name resd 1
    .apm_table       resd 1
    .vbe_control_info resd 1
    .vbe_mode_info   resd 1
    .vbe_mode        resw 1
    .vbe_interface_seg resw 1
    .vbe_interface_off resw 1
    .vbe_interface_len resw 1
endstruc

; Memory map entry
struc MMapEntry
    .size            resd 1
    .base_addr_low   resd 1
    .base_addr_high  resd 1
    .length_low      resd 1
    .length_high     resd 1
    .type           resd 1
endstruc

; =============================================================================
; Detect memory size for Multiboot
; Returns: EAX = lower memory (KB), EDX = upper memory (KB)
; =============================================================================
global multiboot_detect_memory
multiboot_detect_memory:
    push ebx

    ; Use BIOS int 0x15, AH=0x88 for memory size
    mov ah, 0x88
    int 0x15

    ; AX = memory above 1MB in KB
    movzx eax, ax

    ; EDX = 0 for now (simplified)
    xor edx, edx

    pop ebx
    ret

; =============================================================================
; Get memory map using BIOS
; Input: EDI = buffer for memory map
; Output: ECX = number of entries
; =============================================================================
global multiboot_get_memory_map
multiboot_get_memory_map:
    pushad

    ; Set up buffer
    mov [mmap_buffer], edi

    ; Use BIOS int 0x15, EAX=0xE820
    xor ebx, ebx                    ; Continuation value
    xor ecx, ecx                    ; Entry count

.mmap_loop:
    mov eax, 0xE820
    mov edx, 0x534D4150            ; 'SMAP'
    mov edi, [mmap_buffer]
    add edi, ecx
    mov edi, [mmap_buffer]
    imul ecx, ecx, MMapEntry_size
    add edi, ecx

    mov ecx, 24                     ; Entry size
    push ebx
    int 0x15
    pop ebx

    jc .done                       ; Error or done

    test eax, eax
    jz .done

    ; Check for 'SMAP' signature
    cmp edx, 0x534D4150
    jne .done

    inc word [mmap_entry_count]

    ; Check continuation
    test ebx, ebx
    jnz .mmap_loop

.done:
    movzx ecx, word [mmap_entry_count]
    popad
    ret

mmap_buffer:        dd 0
mmap_entry_count:  dw 0

; =============================================================================
; Set up Multiboot info structure
; Input: ESI = kernel command line
; Output: EDI = pointer to Multiboot info
; =============================================================================
global multiboot_setup_info
multiboot_setup_info:
    pushad

    ; Allocate Multiboot info structure
    mov edi, multiboot_info

    ; Set flags
    mov eax, MB_FLAG_MEM | MB_FLAG_CMDLINE | MB_FLAG_MMAP | MB_FLAG_VBE
    mov [edi + MultibootInfo.flags], eax

    ; Get memory info
    call multiboot_detect_memory
    mov [edi + MultibootInfo.mem_lower], eax
    mov [edi + MultibootInfo.mem_upper], edx

    ; Set boot device (simplified - hardcoded)
    mov dword [edi + MultibootInfo.boot_device], 0x80FF0000

    ; Set command line
    test esi, esi
    jz .no_cmdline
    mov [edi + MultibootInfo.cmdline], esi
.no_cmdline:

    ; Get memory map
    lea esi, [edi + 1024]          ; Buffer after info struct
    mov [mmap_buffer], esi
    call multiboot_get_memory_map

    mov esi, [mmap_buffer]
    mov [edi + MultibootInfo.mmap_addr], esi

    ; Calculate total size
    movzx eax, word [mmap_entry_count]
    imul eax, eax, MMapEntry_size
    mov [edi + MultibootInfo.mmap_length], eax

    ; Set VBE info if in graphics mode
    cmp byte [graphics_available], 1
    jne .no_vbe

    mov eax, [framebuffer_addr]
    mov [edi + MultibootInfo.vbe_mode_info], eax

    mov word [edi + MultibootInfo.vbe_mode], 0x010F  ; 800x600x32
.no_vbe:

    ; Set boot loader name
    mov esi, multiboot_loader_name
    mov [edi + MultibootInfo.boot_loader_name], esi

    popad
    ret

; =============================================================================
; Boot kernel with Multiboot
; Input: ESI = kernel entry point, EDI = Multiboot info
; =============================================================================
global multiboot_boot_kernel
multiboot_boot_kernel:
    pushad

    ; Set up registers for kernel
    mov ebx, edi            ; Multiboot info
    mov eax, 0x2BADB002     ; Multiboot magic

    ; Disable interrupts
    cli

    ; Jump to kernel
    jmp esi

    ; Should not return
    hlt

; =============================================================================
; Data section
; =============================================================================
SECTION .data

multiboot_info:
    times MultibootInfo_size db 0

multiboot_loader_name: db "DIT-OS Bootloader", 0

; Multiboot header (for kernel to find)
global multiboot_header
multiboot_header:
    dd MULTIBOOT_HEADER_MAGIC
    dd MULTIBOOT_HEADER_FLAGS
    dd MULTIBOOT_HEADER_CHECKSUM

; Variables for external access
global mb_info_ptr
mb_info_ptr: dd multiboot_info
