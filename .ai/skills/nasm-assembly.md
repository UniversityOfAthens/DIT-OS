# NASM Assembly Coding Guidelines

## Scope

Applies to all `.asm` files across the project: bootloader stage1/stage2, kernel entry, CPU setup, context switching, CRT0.

## Assembler Configuration

```makefile
nasm -f elf32 -g -F dwarf input.asm -o output.o
```

| Flag | Purpose |
|------|---------|
| `-f elf32` | 32-bit ELF object format |
| `-g -F dwarf` | Debug symbols for GDB |
| `-w+all` | Enable all warnings |

## File Structure

```nasm
; DIT-OS <component> - <description>
;
; Brief description of what this file does.

; --- Constants ---
KERNEL_BASE equ 0x100000

; --- Data section ---
section .data
align 4
some_data:
    dd 0x12345678

; --- BSS section ---
section .bss
align 16
kernel_stack:
    resb 8192           ; 8 KiB stack

; --- Code section ---
section .text
global function_name
function_name:
    ; ...
    ret
```

## Section Conventions

| Section | Usage |
|---------|-------|
| `.text` | Executable code |
| `.data` | Initialized data (read-write) |
| `.rodata` | Read-only constants |
| `.bss` | Uninitialized data (zeroed at load) |

For the bootloader MBR (stage1):

```nasm
section .mbr
; 512-byte MBR code
```

## Naming Conventions

### Labels

- **Global functions**: `snake_case`, exported with `global`
- **Local labels**: `.local_label` (prefixed with dot)
- **Constants**: `UPPER_SNAKE_CASE` with module prefix
- **Data labels**: `snake_case`

```nasm
global gdt_install
gdt_install:
    lgdt [gdtr]
    .reload_segments:
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        jmp 0x08:.reload_cs
    .reload_cs:
    ret
```

### Constants

```nasm
; Hardware constants
PIC_MASTER_CMD  equ 0x20
PIC_MASTER_DATA equ 0x21

; Kernel constants
PAGE_SIZE       equ 4096
KERNEL_STACK    equ 0x90000
```

## Calling Convention

### Kernel/Internal Functions (cdecl or custom)

```nasm
; Caller pushes args right to left, caller cleans stack
push arg3
push arg2
push arg1
call function_name
add esp, 12          ; clean 3 args (4 bytes each)

; Frame setup (optional, recommended for non-trivial functions)
function_name:
    push ebp
    mov ebp, esp
    sub esp, locals_size

    ; [ebp+8]  = arg1
    ; [ebp+12] = arg2
    ; [ebp+16] = arg3

    ; ...
    leave
    ret
```

### Syscall Interface (int 0x80)

```nasm
; EAX = syscall number
; EBX = arg1, ECX = arg2, EDX = arg3
; Return in EAX

syscall_0:
    push ebx
    mov eax, [esp+8]   ; syscall number
    int 0x80
    pop ebx
    ret

syscall_1:
    push ebx
    mov eax, [esp+8]   ; syscall number
    mov ebx, [esp+12]  ; arg1
    int 0x80
    pop ebx
    ret
```

## Context Switching

```nasm
; Save all registers, switch stack pointer, restore new context
context_switch:
    pushf
    pushad

    ; Save current stack pointer
    mov [edi], esp      ; edi = &old_task->kstack

    ; Switch to new stack pointer
    mov esp, [esi]      ; esi = &new_task->kstack

    ; Restore registers
    popad
    popf
    ret
```

### Interrupt Frame Layout

When entering an interrupt handler:

```nasm
; Stack layout after automatic save + pusha:
; [esp+48] SS, ESP, EFLAGS, CS, EIP  (auto-saved by CPU on interrupt)
; [esp+28] DS, ES, FS, GS            (manual save)
; [esp+24] EDI, ESI, EBP, dummy      (pushad)
; [esp+8]  EBX, EDX, ECX, EAX        (pushad)
; [esp+4]  interrupt number           (pushed by handler)
; [esp+0]  error code                 (pushed by handler or dummy)
```

## Bootloader Specific

### Stage 1 (MBR)

```nasm
; 512 bytes, loaded at 0x7C00 by BIOS
; Must set up own segments/stack
; Must end with MBR signature

    bits 16
    org 0x7C00

    ; ...

    times 510 - ($ - $$) db 0
    dw 0xAA55           ; MBR signature
```

### Stage 2

```nasm
    bits 16
    ; Loaded at 0x0500 (or wherever stage1 loads it)
    ; Responsibilities:
    ;   1. Enable A20 line
    ;   2. Set up GDT
    ;   3. Switch to protected mode (CR0.PE = 1)
    ;   4. Jump to 32-bit code
    ;   5. Load kernel ELF
```

## Hardware I/O

### Port I/O

```nasm
; Output byte to port
outb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]    ; port
    mov al, [ebp+12]   ; value
    out dx, al
    pop ebp
    ret

; Input byte from port
inb:
    push ebp
    mov ebp, esp
    mov dx, [ebp+8]    ; port
    in al, dx
    xor ah, ah          ; zero-extend to 32-bit
    pop ebp
    ret

; Wait for I/O (short delay)
io_wait:
    out 0x80, al
    ret
```

## Multiboot Header

For GRUB-compatible multiboot (if used):

```nasm
; Must be in the first 8 KiB of the kernel ELF
MULTIBOOT_MAGIC     equ 0x1BADB002
MULTIBOOT_FLAGS     equ 0x00000003
MULTIBOOT_CHECKSUM  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM
```

## Macro Usage

Use macros sparingly and prefix with module name:

```nasm
; Good
%macro PUSHA 0
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi
%endmacro

; Bad (too generic)
%macro PUSH 0
    ...
%endmacro
```

## Common Patterns

### Disable Interrupts / Restore

```nasm
    pushf
    pop eax
    test eax, 0x200         ; IF flag
    jz .already_off
    cli
```

### Stack Alignment

Always align stack to 16 bytes before `call` (for SSE):

```nasm
    and esp, -16            ; align to 16 bytes
    call kernel_main
```

## Absolute Rules

1. **Always specify `bits 16` or `bits 32`** - do not rely on section defaults
2. **Never use `times` in code sections** - only for padding/filling
3. **Document every hardware port access** - include port number and purpose
4. **Never clobber callee-saved registers without saving** - EBX, ESI, EDI, EBP, ESP
5. **Always include MBR signature** (0xAA55) in stage1
6. **Always set up GDT before protected mode switch**
7. **Always load kernel ELF at correct address before jump**
8. **Never assume register state on interrupt entry** - save everything
