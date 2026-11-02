---
name: bootloader
description: The DIT-OS custom boot chain (stage1 MBR, stage2 menu, loader, kernel handoff), the disk image layout and the files that must stay in sync, what is dead code, and boot-time safety rules. Load before editing bootloader/.
---

# Bootloader

## Boot chain

1. **Stage1** (`src/stage1/stage1.asm`): 512-byte MBR at `0x7C00`; prints
   `[1/3] Stage1...`, reads stage2.
2. **Stage2** (`src/stage2/stage2.asm`): real mode at `0x7E00`; prints
   `[2/3] Stage2 loaded.`, shows the boot menu (waits for Enter), reads the
   loader and kernel image (`[3/3] Loading loader+kernel...`, `Read OK.`).
3. **Loader** (`src/loader/loader.asm`): at `0x10000`; enters protected mode,
   parses the kernel ELF from `0x20000`, copies segments to `0x100000`,
   jumps to `kernel_entry`. Prints `L`, `O`, `E`, `K`.

## Disk layout (`bootloader/Makefile` is the owner)

| Sectors | Content | Load address |
|---------|---------|--------------|
| 0 | stage1.bin | 0x7C00 |
| 2–10 | stage2.bin (9 sectors) | 0x7E00 |
| 11–36 | loader.bin (26 reserved) | 0x10000 |
| 37+ | kernel.img, `KERNEL_SECTORS` (200) | raw at 0x20000, segments to 0x100000 |

- `kernel.img` is the stripped `kernel.bin`. The build fails if it exceeds
  `KERNEL_SECTORS * 512`.
- `KERNEL_SECTORS` is passed to `stage2.asm` with `-D`.
- When any of these numbers change, update **all** of: `bootloader/Makefile`,
  `stage1.asm` (stage2 CHS read), `stage2.asm` (loader/kernel LBAs),
  `src/common/constants.inc`, `bootloader/include/boot.hpp`, the disk-layout
  check in `.github/workflows/ci.yml`, and the tables in `AGENTS.md` and
  this skill.

## What is built

- Built: `stage1.asm`, `stage2.asm`, `loader.asm` (NASM only).
- **Not built (dead code):** the C++ files in `src/stage2/`, `src/uefi/`
  (`ISSUES.md` H18). Do not "fix" them as part of other work, and do not
  break `make -C bootloader all` by wiring them in half-way.
- `boot/grub/` and `tools/build-iso/` are for the optional ISO path, not the
  main boot.
- From `src/stage2/*.cpp`, includes are `#include "../../include/foo.hpp"`.

## Make targets

`all` (disk image), `run` (interactive QEMU), `debug` (QEMU + GDB on :1234),
`clean`. The kernel must be built first (`make -C kernel`).

## Safety rules

1. No dynamic allocation; fixed buffers with known addresses only.
2. Check the carry flag after every BIOS disk call; retry, then halt with a
   message. Never continue with a partial read.
3. Bound every sector count and every ELF field before using it
   (`e_phoff`, `e_phnum`, `p_offset + p_filesz`, `p_vaddr + p_memsz`).
4. Buffers must not overlap the stack, the IVT/BDA (`0x0000`–`0x04FF`),
   stage1/stage2, or the kernel staging area.
5. Menu / keyboard input is bounded by its buffer (BootHole class,
   CVE-2020-10713).
6. Set up the GDT before the protected-mode switch; far-jump to flush CS.
7. The kernel's entry contract (registers, load address, multiboot info) is
   defined by the loader and `kernel/arch/x86_64/boot/entry.asm`; change
   both together.
