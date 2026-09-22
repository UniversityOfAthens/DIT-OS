; =============================================================================
; DIT-OS UEFI Bootloader
; PE/COFF executable for UEFI
; =============================================================================

; DOS stub - required for PE header
SECTION .dos_stub
org 0
dos_stub:
    ; Minimal DOS program that prints error and exits
    db 'This program cannot be run in DOS mode.', 0

; PE Header
SECTION .pe_header
pe_header:
    ; PE Signature
    db 'PE', 0, 0

    ; COFF Header
coff_header:
    dw 0x8664      ; Machine = AMD64 (x86-64)
    dw 2           ; NumberOfSections
    dd 0           ; TimeDateStamp
    dd 0           ; PointerToSymbolTable
    dd 0           ; NumberOfSymbols
    dw 0xE0        ; SizeOfOptionalHeader (PE32+)
    dw 0x202       ; Characteristics (Executable, Large address aware)

    ; Optional Header (PE32+)
optional_header:
    dw 0x20B       ; Magic = PE32+ (64-bit)
    db 14          ; MajorLinkerVersion
    db 0           ; MinorLinkerVersion
    dd 0           ; SizeOfCode
    dd 0           ; SizeOfInitializedData
    dd 0           ; SizeOfUninitializedData
    dd 0           ; AddressOfEntryPoint (set later)
    dd 0           ; BaseOfCode
    dd 0           ; BaseOfData
    dq 0x100000000 ; ImageBase (4GB)
    dd 0x1000      ; SectionAlignment
    dd 0x200       ; FileAlignment
    dw 5           ; MajorOperatingSystemVersion
    dw 0           ; MinorOperatingSystemVersion
    dw 0           ; MajorImageVersion
    dw 0           ; MinorImageVersion
    dw 6           ; MajorSubsystemVersion (UEFI)
    dw 0           ; MinorSubsystemVersion
    dd 0           ; Win32VersionValue
    dd 0x30000     ; SizeOfImage (192KB)
    dd 0x200       ; SizeOfHeaders
    dd 0           ; CheckSum
    dw 10          ; Subsystem = 10 (EFI Application)
    dw 0           ; DllCharacteristics
    dq 0x100000    ; SizeOfStackReserve
    dq 0x1000      ; SizeOfStackCommit
    dq 0x100000    ; SizeOfHeapReserve
    dq 0           ; SizeOfHeapCommit
    dd 0           ; LoaderFlags
    dd 16          ; NumberOfRvaAndSizes

    ; Data directories
    dd 0, 0        ; Export Table
    dd 0, 0        ; Import Table
    dd 0, 0        ; Resource Table
    dd 0, 0        ; Exception Table
    dd 0, 0        ; Certificate Table
    dd 0, 0        ; Base Relocation Table
    dd 0, 0        ; Debug Table
    dd 0, 0        ; Architecture
    dd 0, 0        ; Global Ptr
    dd 0, 0        ; TLS Table
    dd 0, 0        ; Load Config Table
    dd 0, 0        ; Bound Import
    dd 0, 0        ; IAT
    dd 0, 0        ; Delay Import
    dd 0, 0        ; COM Descriptor
    dd 0, 0        ; Reserved

; Section 1: .text
section_header_1:
    db '.text'
    db 0, 0, 0     ; Name (8 bytes)
    dd 0           ; VirtualSize
    dd 0x1000      ; VirtualAddress
    dd 0           ; SizeOfRawData
    dd 0           ; PointerToRawData
    dd 0           ; PointerToRelocations
    dd 0           ; PointerToLinenumbers
    dw 0           ; NumberOfRelocations
    dw 0           ; NumberOfLinenumbers
    dd 0x60000020  ; Characteristics (CODE | EXECUTE | READ)

; Section 2: .data
section_header_2:
    db '.data'
    db 0, 0, 0     ; Name (8 bytes)
    dd 0           ; VirtualSize
    dd 0x2000      ; VirtualAddress
    dd 0           ; SizeOfRawData
    dd 0           ; PointerToRawData
    dd 0           ; PointerToRelocations
    dd 0           ; PointerToLinenumbers
    dw 0           ; NumberOfRelocations
    dw 0           ; NumberOfLinenumbers
    dd 0xC0000040  ; Characteristics (INITIALIZED_DATA | READ | WRITE)

; Section 3: .reloc (required for base relocations)
section_header_3:
    db '.reloc'
    db 0, 0, 0     ; Name (8 bytes)
    dd 0           ; VirtualSize
    dd 0x3000      ; VirtualAddress
    dd 0           ; SizeOfRawData
    dd 0           ; PointerToRawData
    dd 0           ; PointerToRelocations
    dd 0           ; PointerToLinenumbers
    dw 0           ; NumberOfRelocations
    dw 0           ; NumberOfLinenumbers
    dd 0x42000040  ; Characteristics (INITIALIZED_DATA | READ | DISCARDABLE)

; Padding to 0x200 (512 bytes) for headers
times 0x200 - ($ - $$) db 0

; =============================================================================
; UEFI Application Code
; =============================================================================
SECTION .text

[BITS 64]

; Entry point - called by UEFI
global efi_main
efi_main:
    ; Parameters:
    ; rcx = ImageHandle (EFI_HANDLE)
    ; rdx = SystemTable (EFI_SYSTEM_TABLE*)

    ; Save system table
    mov [system_table], rdx

    ; Get boot services
    mov rax, [rdx + 48]    ; SystemTable->BootServices
    mov [boot_services], rax

    ; Initialize console output
    call init_conout

    ; Print welcome
    lea rsi, [welcome_msg]
    call efi_puts

    ; Initialize graphics
    call init_graphics

    ; Load boot config
    call load_boot_config

    ; Display boot menu
    call display_boot_menu

    ; Load kernel
    call load_kernel

    ; Exit boot services
    call exit_boot_services

    ; Jump to kernel
    jmp kernel_entry

; =============================================================================
; Initialize console output
; =============================================================================
init_conout:
    push rbx

    mov rax, [system_table]
    mov rbx, [rax + 64]   ; ConOut handle
    mov rax, [rax + 72]   ; SimpleTextOutputProtocol

    mov [conout_handle], rbx
    mov [conout_protocol], rax

    ; Clear screen
    mov rcx, rbx
    mov rdx, rax
    xor r8, r8              ; Attribute = default
    call [rax + 8]         ; ClearScreen

    pop rbx
    ret

; =============================================================================
; Print string to UEFI console
; Input: RSI = string
; =============================================================================
efi_puts:
    push rcx
    push rdx

    mov rcx, [conout_handle]
    mov rdx, rsi

    mov rax, [conout_protocol]
    call [rax + 6]         ; OutputString

    pop rdx
    pop rcx
    ret

; =============================================================================
; Initialize graphics mode
; =============================================================================
init_graphics:
    push rbx

    ; Get GOP
    mov rax, [boot_services]
    mov rbx, [rax + 56]    ; LocateProtocol GOP

    ; Note: In full implementation, would locate GOP and set mode

    pop rbx
    ret

; =============================================================================
; Load boot configuration
; =============================================================================
load_boot_config:
    push rsi

    lea rsi, [config_msg]
    call efi_puts

    ; Parse /boot/bootloader/config/boot.cfg
    ; For now, use defaults

    pop rsi
    ret

; =============================================================================
; Display boot menu
; =============================================================================
display_boot_menu:
    push rsi

    lea rsi, [menu_msg]
    call efi_puts

    ; In full implementation, would:
    ; 1. Read entries from config
    ; 2. Display menu
    ; 3. Wait for input
    ; 4. Return selected entry

    pop rsi
    ret

; =============================================================================
; Load kernel into memory
; =============================================================================
load_kernel:
    push rsi

    lea rsi, [loading_msg]
    call efi_puts

    ; In full implementation:
    ; 1. Read kernel from filesystem
    ; 2. Parse ELF
    ; 3. Load into memory
    ; 4. Set up Multiboot info

    pop rsi
    ret

; =============================================================================
; Exit boot services
; =============================================================================
exit_boot_services:
    push rax
    push rcx

    mov rax, [boot_services]
    mov rcx, [image_handle]

    ; Get memory map size first
    xor ebx, ebx

    ; Call ExitBootServices(ImageHandle, MapKey)
    ; Note: Need proper memory map for this

    pop rcx
    pop rax
    ret

; =============================================================================
; Data section
; =============================================================================
SECTION .data

image_handle:     dq 0
system_table:     dq 0
boot_services:    dq 0
conout_handle:   dq 0
conout_protocol: dq 0
kernel_entry:     dq 0
kernel_size:      dq 0
kernel_load_addr: dq 0

welcome_msg: db "DIT-OS UEFI Bootloader", 13, 10, 0
config_msg:  db "Loading configuration...", 13, 10, 0
menu_msg:    db "Boot Menu:", 13, 10, 0
loading_msg: db "Loading kernel...", 13, 10, 0

; =============================================================================
; Relocation data (required for ASLR)
; =============================================================================
SECTION .reloc

reloc_base:
    dd 0           ; Page RVA
    dd 0           ; Block Size
    ; Relocation entries would go here
