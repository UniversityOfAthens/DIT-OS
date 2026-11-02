---
name: nasm-assembly
description: NASM conventions for DIT-OS boot code, kernel entry, interrupt stubs and context switching — BITS, sections, output formats, calling convention, register preservation. Load before editing any .asm or .inc file.
---

# NASM assembly

## Formats

| Code | Format | Notes |
|------|--------|-------|
| stage1, stage2, loader | `nasm -f bin` | Flat binaries, `ORG` set in the file |
| Kernel asm, `crt0.asm` | `nasm -f elf32 -g` | Linked with C/C++ objects |

Never produce `elf64` objects; never mix 64-bit code into the 32-bit build.

## File shape

```nasm
; DIT-OS <component> - <purpose>

bits 32

KERNEL_STACK_SIZE equ 16384

section .bss
align 16
kernel_stack:
    resb KERNEL_STACK_SIZE

section .text
global some_function
some_function:
    push ebp
    mov ebp, esp
    ; ...
    pop ebp
    ret
```

- Every assembled file states `bits 16` or `bits 32` (CI checks the files
  that are built). Switch to `bits 32` only after entering protected mode.
- Sections: `.text`, `.data`, `.rodata`, `.bss`; align data the hardware
  or ABI requires (GDT 8, IDT 8, stacks 16, page tables 4096).
- Shared bootloader constants live in `bootloader/src/common/constants.inc`.
  No magic addresses inline.
- `times` is for padding only (e.g. the MBR fill), never to generate code.

## Naming

- Exported functions: `snake_case`, `global`, no leading underscore (ELF).
- Local labels: `.label`.
- Constants: `UPPER_SNAKE_CASE` with a module prefix.

## Calling convention (cdecl, i386)

- Args on the stack right to left; caller cleans up; `[ebp+8]` = arg 1.
- Return in EAX (EDX:EAX for 64-bit).
- Callee-saved: EBX, ESI, EDI, EBP, ESP. Save them if you touch them.
- Direction flag clear on call and return (`cld`).

## Interrupt / exception / syscall stubs

- Save **all** general registers and segment registers on entry; assume
  nothing about the interrupted state.
- Load kernel data segments before calling C++.
- Exceptions with and without an error code must produce the same frame
  (push a dummy 0 where the CPU does not).
- Return to ring 3 only through a frame the kernel built or validated:
  user CS/SS with RPL 3, EFLAGS with IOPL 0 and IF set.
- Send EOI for hardware IRQs in the right place (PIC/APIC).
- Update TSS `esp0` when switching tasks.

## Context switch

`kernel/sched/context.asm` saves callee-saved registers and EFLAGS, swaps
ESP between task kernel stacks and returns into the new task. Any change
here must keep the saved-frame layout identical to what `fork` and task
creation build on a new stack. Test with the fork/execve self-tests.

## Bootloader specifics

- Stage1: `bits 16`, `ORG 0x7C00`, exactly 512 bytes, `dw 0xAA55` at 510.
- Stage2: `bits 16`, `ORG 0x7E00`; enables A20, sets up the GDT, switches to
  protected mode.
- Loader: `[org 0]`, loaded at `0x10000`, parses the kernel ELF and copies
  its segments to `0x100000`.
- Check the BIOS carry flag after every `int 0x13` and handle errors.

## Rules

1. Document every port, MSR and control-register bit you touch.
2. Never clobber callee-saved registers without restoring them.
3. Keep stack alignment consistent before calling C/C++.
4. Comment non-obvious steps (A20, paging enable, far jumps).
