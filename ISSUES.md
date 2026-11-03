# DIT-OS Issue Tracker

This file catalogs all known issues across the codebase, organized by severity.
Generated from a comprehensive audit of kernel, bootloader, userland, build system, and configuration.
Fixed issues are removed from this file; see `CHANGELOG.md` for what was fixed and when.

**Note on bootloader modules**: the active build assembles only `stage1.asm`,
`stage2.asm` and `loader.asm`. The other files in `bootloader/src/stage2/`
(`a20.asm`, `disk.asm`, `png.asm`, `memory.asm`, ...) and `bootloader/src/uefi/`
are not assembled, so their bugs cannot affect the current boot (see H18).

---

## CRITICAL — Will cause boot failure, crashes, or compile errors

### C9. PNG decoder header chunk lookup always fails

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/png.asm` |
| **Lines** | 38 (never-initialized `png_target_chunk`) |
| **Issue** | `png_target_chunk` is never initialized (remains 0), so `png_find_chunk` can never find any chunk. Additionally, `zlib_decompress` at line 191 is a stub that does nothing. PNG images are never decoded. |
| **Fix** | Implement proper chunk-type matching and a working zlib decompressor, or remove the dead code. |
| **Note** | `png.asm` is not assembled into the current build, so this cannot break boot today. |

### C13. Disk LBA segment hardcoded to 0

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/disk.asm` |
| **Line** | 47 |
| **Issue** | `mov word [si+6], 0` always sets the buffer segment to 0 for LBA reads, ignoring any caller-specified segment. Data may be written to the wrong memory location if the caller expects a different segment. |
| **Fix** | Use the segment value passed by the caller, or document that the segment is always 0. |
| **Note** | `disk.asm` is not assembled into the current build; `stage2.asm` has its own `read_lba`. |

---

## HIGH — Will cause build failures, incorrect behavior, or significant confusion

### H11. 20+ duplicate global symbols in bootloader ELF build path

| Field | Value |
|-------|-------|
| **Files** | `bootloader/src/stage2/stage2_core.asm` + many individual `.asm` files |
| **Issue** | Symbols like `print_string`, `clear_screen`, `font_color`, `draw_char`, `plot_pixel`, `check_vesa`, `load_theme`, etc. are defined in both `stage2_core.asm` and their respective module `.asm` files. Any attempt to link the ELF build will fail with duplicate symbol errors. |
| **Fix** | Consolidate symbol definitions: each symbol should be defined in exactly one place. |

### H12. C++ bootloader source files have broken include paths

| Field | Value |
|-------|-------|
| **Files** | `bootloader/src/stage2/gdt.cpp:5`, `src/stage2/paging.cpp:5` |
| **Issue** | `#include "../include/types.hpp"` resolves relative to `src/stage2/`, producing `src/include/types.hpp` which doesn't exist. Should be `../../include/types.hpp`. |
| **Fix** | Fix the include paths, or remove the files if the C++ build path is not used. |

### H13. AHCI driver uses port I/O instead of MMIO

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/dev/ahci.cpp` |
| **Lines** | 13-15 |
| **Issue** | AHCI BAR5 is an MMIO region, not an I/O port. The code treats BAR5 as a port base and uses `arch::inl()` to read it, which will read garbage or fault. |
| **Fix** | Map BAR5's physical address and access it via `volatile uint32_t*` pointers. |

### H14. AHCI/NVMe/Floppy drivers are empty stubs

| Field | Value |
|-------|-------|
| **Files** | `kernel/arch/x86_64/dev/ahci.cpp`, `nvme.cpp`, `floppy.cpp` |
| **Issue** | These files return `-1` from their init functions immediately. No actual driver logic is implemented. |
| **Fix** | Implement proper drivers or remove the stubs and add `-ENODEV` returns. |

### H18. Dead C++ stage2 build path not invoked by any Makefile

| Field | Value |
|-------|-------|
| **Files** | All `.cpp` and `.hpp` files in `bootloader/src/stage2/` |
| **Issue** | `bootloader/src/stage2/Makefile` only builds the flat binary from `stage2.asm`. The C++ files (`gdt.cpp`, `paging.cpp`, `stage2.cpp`, etc.) are never compiled or linked. The `stage2.ld` linker script exists but is unused. This is misleading dead code. |
| **Fix** | Either wire up the ELF-linked C++ build path or remove the unused files. |

### H20. `loader.asm` hardcodes 18 sectors/track, 2 heads (CHS geometry)

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/stage2.asm` (and `loader.asm`) |
| **Lines** | 422-428 |
| **Issue** | CHS geometry is hardcoded to 18 sectors/track and 2 heads. Works for 1.44 MB floppy images but will fail for hard disks or USB drives with different geometry. No LBA fallback is implemented. |
| **Fix** | Use LBA for all disk access, or detect CHS geometry from the BIOS (int 0x13, AH=8) before attempting CHS addressing. |

---

## MEDIUM — Important but not blocking

### M1. Widespread use of raw `int` instead of sized types

| Field | Value |
|-------|-------|
| **Files** | Hundreds of locations across `core/printk.cpp`, `boot/main.cpp`, `console.cpp`, `vfs.cpp`, `ramfs.cpp`, `pci.cpp`, `ata.cpp`, `floppy.cpp`, `keyboard.cpp`, `sys_read.cpp`, `sys_write.cpp`, `fd.cpp` etc. |
| **Issue** | Raw `int`, `unsigned int`, `long` are used instead of `u32`/`s32`/`u16` as required by AGENTS.md type conventions. |
| **Fix** | Replace with properly-sized types per `kernel/include/types.hpp`. |

### M4. Duplicated `kstrcmp` in VFS and ramfs

| Field | Value |
|-------|-------|
| **Files** | `kernel/fs/vfs.cpp:25-30`, `kernel/fs/ramfs/ramfs.cpp:47-55` |
| **Issue** | The `kstrcmp` function is copy-pasted in two places. Code duplication. |
| **Fix** | Move to a shared header (`kernel/include/string.hpp`) or implement `strcmp` via the libk. |

### M5. Duplicate VGA putchar code in `printk.cpp` and `console.cpp`

| Field | Value |
|-------|-------|
| **Files** | `kernel/core/printk.cpp:15-35`, `kernel/core/console.cpp:15-35` |
| **Issue** | Both files contain an identical VGA putchar implementation (scroll handler, cursor positioning, VGA buffer write). The `printk.cpp` version is separate from the console layer's version. |
| **Fix** | Have `printk.cpp` delegate to `console.cpp` functions instead of duplicating them. |

### M6. `config_init()` is a no-op

| Field | Value |
|-------|-------|
| **File** | `kernel/core/config.cpp` |
| **Lines** | 11-13 |
| **Issue** | `config_init()` takes no action. Config struct `s_config` has reasonable defaults from its initializer but is never populated from the command line or boot info. |
| **Fix** | Implement actual config parsing from kernel command line or bootloader info. |

### M7. Drivers return bare `-1` instead of errno values

| Field | Value |
|-------|-------|
| **Files** | `kernel/arch/x86_64/dev/ata.cpp`, `ahci.cpp`, `nvme.cpp`, `floppy.cpp`, `serial.cpp`, `kernel/arch/x86_64/mm/vmm.cpp` |
| **Issue** | The syscall layer now uses symbolic errno values, but drivers and the VMM still return `-1`, which callers cannot distinguish (`-EPERM` by accident). |
| **Fix** | Return specific negative errno values (`-ENODEV`, `-EIO`, `-EINVAL`, ...). |

### M8. Missing `DT_*` dirent type enum in `namespace fs`

| Field | Value |
|-------|-------|
| **File** | `kernel/include/fs/dirent.hpp` |
| **Lines** | 18-26 |
| **Issue** | `DT_UNKNOWN`, `DT_FIFO`, etc. are C-style `#define` macros polluting the global namespace instead of being in `namespace fs`. |
| **Fix** | Move to `namespace fs { enum DirentType { ... }; }` or use `constexpr` constants. |

### M9. Unsigned/signed mixing in `vfs_lseek`

| Field | Value |
|-------|-------|
| **File** | `kernel/fs/vfs.cpp` |
| **Lines** | 200-210 |
| **Issue** | Casts between `int64_t` and `uint64_t` for file positions. Adding `int64_t offset` to `uint64_t f_pos` has implicit conversion rules that may produce unexpected results for negative offsets. |
| **Fix** | Use signed file positions throughout or add explicit overflow checks. |

### M10. `ata_read_sectors` missing upper-bound check

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/dev/ata.cpp` |
| **Line** | 48 |
| **Issue** | Only checks `num == 0` but doesn't validate that the sector count is ≤ 255 (per ATA single-command limit). Oversized requests cause excessive loop iterations. |
| **Fix** | Add `if (num == 0 || num > 255) return -EINVAL;` and handle 0=256 per ATA spec. |

### M13. `scripts/create-disk.sh` uses magic numbers for disk layout

| Field | Value |
|-------|-------|
| **File** | `scripts/create-disk.sh` |
| **Lines** | 16-21 |
| **Issue** | Sector counts and offsets (2880, 2, 11, 37) are hardcoded instead of using named constants. Must stay in sync with `bootloader/Makefile`, `constants.inc`, and `boot.hpp`. |
| **Fix** | Derive values from a shared configuration source (e.g., a `disk_layout.sh` that reads from `boot.hpp`). |

### M15. Missing include directory referenced in build flags

| Field | Value |
|-------|-------|
| **File** | `kernel/Makefile:79`, `CMakeLists.txt:49` |
| **Issue** | Both files add `-I../include` (or `-I${CMAKE_SOURCE_DIR}/include`), but the directory `/home/qwerty/Desktop/Github/DIT-OS/include/` **does not exist**. The include path is silently ignored. |
| **Fix** | Create the directory or remove the include flag. |

### M16. `arch/x86_64/` is a misnomer — all code is 32-bit

| Field | Value |
|-------|-------|
| **Directory** | `kernel/arch/x86_64/` |
| **Issue** | The entire kernel compiles with `-m32`, uses 32-bit paging, 32-bit GDT/IDT entries, and links as `elf32-i386`. The directory is named `x86_64` which is misleading. AGENTS.md acknowledges this as a known misnomer. |
| **Fix** | Rename `arch/x86_64/` to `arch/i386/` or `arch/x86/` throughout the project. |

### M17. No userland `errno.h` or `errno` variable

| Field | Value |
|-------|-------|
| **Directory** | `userland/` |
| **Issue** | POSIX requires `<errno.h>` with an `errno` variable (or thread-local) for inspecting syscall error codes. Neither exists in userland. Syscall errors are invisible to user programs. |
| **Fix** | Add `userland/libc/include/errno.h` with extern `int errno` declaration and the errno constant definitions. |

### M18. Userland error returns silently swallowed in shell

| Field | Value |
|-------|-------|
| **File** | `userland/shell/main.c` |
| **Lines** | 228-229, 256-257, 284-286 |
| **Issue** | `getdents()` and `read()` error returns (negative values) are treated identically to "end of data" (0). Errors are silently swallowed — the user has no way to know a read failed. |
| **Fix** | Check for negative return values and report errors to stderr. |

### M19. `_exit()` calls inline `exit()` — fragile coupling

| Field | Value |
|-------|-------|
| **File** | `userland/libc/src/stdlib/exit.c` |
| **Line** | 8 |
| **Issue** | `_exit(code)` calls `exit(code)`, which is a static inline function from `syscall.h`. If `syscall.h` is not included or the inline is removed, `_exit` silently breaks. |
| **Fix** | Have `_exit` call `dit_sys_exit()` directly instead of going through the inline wrapper. |

### M20. PNG decoder truncates 32-bit chunk length to 16 bits

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/png.asm` |
| **Lines** | 129, 136 |
| **Issue** | `movzx ecx, word [png_chunk_length]` truncates 32-bit chunk lengths to 16 bits. Chunks larger than 64 KB (IHDR, IDAT) will overflow and cause incorrect reads. |
| **Fix** | Use full 32-bit `mov ecx, [png_chunk_length]` instead of `movzx` from 16 bits. |

### M21. A20 gate uses only "fast method" — no fallback

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/a20.asm` |
| **Lines** | 7-9 |
| **Issue** | Only port 0x92 (fast A20) is used. No fallback to keyboard controller (method 1) or BIOS int 0x15 AX=0x2401. Some systems do not support the fast A20 method. |
| **Fix** | Add fallback paths for keyboard controller and BIOS A20 methods. |

### M22. Stage2 never enables A20 before passing control to loader

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/stage2.asm` |
| **Issue** | Stage2 stays in real mode and jumps to the loader which enables A20. However, if stage2 needed to access memory above 1MB for any reason (e.g., loading large files with FAT32), A20 would need to be enabled earlier. |
| **Fix** | Enable A20 in stage2 (before the menu code) or document why it's not needed. |

### M23. `uefi.asm` section count mismatch in PE header

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/uefi/uefi.asm` |
| **Line** | 22 |
| **Issue** | COFF header says `dw 2` (2 sections), but there are 3 section headers (.text, .data, .reloc). A PE loader may skip the .reloc section, causing relocation failures. |
| **Fix** | Change `dw 2` to `dw 3` to match the actual number of sections. |

### M24. All UEFI PE section raw data sizes are 0

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/uefi/uefi.asm` |
| **Lines** | 84-106 |
| **Issue** | All section headers have `RawSize = 0` and `VirtualSize = 0`. No actual code or data is produced. The PE file is structurally incomplete. |
| **Fix** | Set proper sizes in the section headers. |

### M25. `memory.asm` C/ASM calling convention mismatch

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/memory.asm` (ASM) vs `bootloader/include/memory.hpp:25` (C++) |
| **Issue** | ASM function expects a FAR pointer (segment:offset) passed via registers `di` (offset) and `bx` (segment). The C declaration `int memory_map_e820(E820Entry *entries, int max_count)` passes a NEAR pointer. The calling conventions don't match. |
| **Fix** | Standardize on one calling convention (preferably NEAR with the flat binary's segment setup). |

### M26. `stage2_main.asm` loads kernel from wrong CHS address

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/stage2_main.asm` |
| **Lines** | 217-219 |
| **Issue** | The ELF build path reads the kernel from CHS sector 6, head 1 (≈ LBA 23), but the Makefile places the kernel at LBA 37. The ELF-linked build path would read the wrong data. |
| **Fix** | Update CHS address or use LBA addressing consistent with the Makefile. |

### M28. `cmd_cat()` uses small 256-byte read buffer

| Field | Value |
|-------|-------|
| **File** | `userland/shell/main.c` |
| **Line** | 254 |
| **Issue** | `char chunk[256]` — adequate for ramfs but would cause many read syscalls for larger files. |
| **Fix** | Increase buffer size or make it a configurable constant. |

### M31. Missing `KERNEL_PACKED` on ABI-boundary structs

| Field | Value |
|-------|-------|
| **Files** | `kernel/include/fs/dirent.hpp`, `kernel/include/fs/stat.hpp` |
| **Issue** | Structs that cross the user/kernel boundary (dirent, stat) lack `__attribute__((packed))` / `KERNEL_PACKED`. If compiler padding differs between kernel and userland, data will be misaligned. |
| **Fix** | Add `KERNEL_PACKED` to all ABI structs. |

### M35. Userland `malloc` never frees memory

| Field | Value |
|-------|-------|
| **File** | `userland/libc/src/stdlib/malloc.c` |
| **Issue** | A 64 KB bump allocator whose `free()` is a no-op, the same bug the kernel heap had. Nothing calls it yet, but any userland program that allocates in a loop will run out. |
| **Fix** | Reuse the kernel heap's free-list design, or implement `brk`/`mmap` and a real allocator. |

### M38. `malloc()` size arithmetic can overflow

| Field | Value |
|-------|-------|
| **File** | `userland/libc/src/stdlib/malloc.c` |
| **Issue** | `align_up(size)` computes `(size + 7) & ~7` without checking for overflow, and `s_used + size` can wrap too. `malloc(0xFFFFFFFF)` therefore returns a pointer with no space behind it, and writing through it overruns `s_heap`. No current caller passes an attacker-controlled size. |
| **Fix** | Reject sizes above `sizeof(s_heap) - s_used` before aligning, or check `size > SIZE_MAX - 7` first. |

### M41. Reparented orphans can accumulate while init is blocked

| Field | Value |
|-------|-------|
| **Files** | `userland/shell/main.c`, `kernel/sched/exit.cpp` |
| **Bump** | patch |
| **Issue** | `task_exit()` reparents orphans to PID 1 and the shell drains them before each prompt, but while the shell is blocked in `read()` a process can keep forking and exiting orphans, accumulating zombies and kernel stacks until the next prompt. Normal use is bounded; a hostile workload is not. |
| **Fix** | Deliver `SIGCHLD` and reap from a handler, or add a kernel reaper/cap for orphan zombies (with the signal work). |

---

## LOW — Cleanup, style, and minor issues

### L1. `userland/crt0.asm` does not clear BSS

| Field | Value |
|-------|-------|
| **File** | `userland/crt0.asm` |
| **Issue** | AGENTS.md says crt0 must clear BSS, set up stack, call main(), then exit(). The current implementation skips BSS clearing entirely. Currently unused, but will cause uninitialized globals when activated. |

### L2. `userland/include/syscall.h` uses `int` instead of `ssize_t` for return types

| Field | Value |
|-------|-------|
| **File** | `userland/include/syscall.h` |
| **Lines** | 28-29, 39-46 |
| **Issue** | POSIX `read()`/`write()` return `ssize_t`. Using `int` could truncate values > 2 GB (theoretically on 32-bit). |

### L3. No `restrict` keyword on userland syscall buffer wrappers

| Field | Value |
|-------|-------|
| **File** | `userland/include/syscall.h` |
| **Lines** | 39-46 |
| **Issue** | POSIX `read(void *restrict buf, ...)` uses `restrict`, but the userland wrappers omit it. Prevents some compiler optimizations. |

### L4. `utoa()` has no output buffer size parameter

| Field | Value |
|-------|-------|
| **File** | `userland/libc/src/stdio/printf.c` |
| **Lines** | 21-32 |
| **Issue** | `utoa()` writes to `out` without knowing buffer capacity. Currently safe (callers pass `num_buf[16]`), but would overflow with a base-2 conversion of `uint32_t` (needs 32 chars) or a smaller output buffer. |

### L5. `boot.hpp` and other bootloader headers sometimes mix `<stdint.h>` and `"types.hpp"`

| Field | Value |
|-------|-------|
| **File** | `bootloader/include/config.hpp:4` |
| **Issue** | Uses `#include <stdint.h>` (angle brackets, system header) while other bootloader headers use `#include "types.hpp"`. Inconsistency. |

### L7. No compile-time assertion for userland/kernel struct layout compatibility

| Field | Value |
|-------|-------|
| **Files** | `userland/include/syscall.h:18-23` |
| **Issue** | `dit_dirent` in userland must match `fs::dirent` in the kernel. There is no `static_assert` or compile-time check for layout compatibility. |

### L8. `pci_scan` uses `uint16_t bus` but limits loop to 256 (fits in `uint8_t`)

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/dev/pci.cpp` |
| **Line** | 49 |
| **Issue** | `uint16_t bus` wastes space. Loop bound is 256, which fits in `uint8_t`. |

### L9. `pci_find_class` reads function 0 config space twice

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/dev/pci.cpp` |
| **Lines** | 66, 68 |
| **Issue** | The outer check reads function 0 to check for device presence, then the inner loop also reads function 0. Double read. |

### L10. Missing `.gitignore` entries

| Field | Value |
|-------|-------|
| **File** | `.gitignore` |
| **Issue** | Missing entries for `*.swp`, `*.swo`, `*~`, `.DS_Store`, `.cache/`, `compile_commands.json`, `.vscode/`, `.idea/`, and `*.d` dependency files outside `build/`. |

### L12. Manual byte-zeroing instead of `memset`

| Field | Value |
|-------|-------|
| **File** | `kernel/fs/vfs.cpp:48` |
| **Issue** | Uses a manual byte-zeroing loop (`for (uint32_t i = 0; i < sizeof(dentry); i++) ((uint8_t*)d)[i] = 0;`) instead of calling a `memset` function. Kernel has `string.hpp` declaring `memset`. |

### L13. Context switch does not save/restore segment registers

| Field | Value |
|-------|-------|
| **File** | `kernel/sched/context.asm` |
| **Issue** | When kernel supports user-mode processes with different segment selectors, DS, ES, FS, GS must be saved/restored. Currently safe because all tasks run in ring 0. |

### L14. Stage2 `dap_segment` usage overwrites segment register assumptions

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/stage2.asm` |
| **Line** | 444-445 |
| **Issue** | The DAP segment increment logic (`add word [dap_segment], 0x1000`) works, but assumes a particular segment setup that may not hold if other code modifies segment registers. |

### L16. `flake.nix` missing formatter and checks

| Field | Value |
|-------|-------|
| **File** | `flake.nix` |
| **Issue** | No `formatter` output (for `nix fmt`) and no `checks` (for `nix flake check`, e.g. build the disk image and boot it in QEMU). |

### L17. No `ENOSYS` in userland errno

| Field | Value |
|-------|-------|
| **Files** | Userland `errno.h` (doesn't exist) |
| **Issue** | Until userland has an errno.h, users cannot programmatically check syscall errors. |

### L19. Stage1 stores the boot drive before setting `DS`

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage1/stage1.asm` |
| **Issue** | `mov [boot_drive], dl` runs before `DS` is zeroed, so on a BIOS that enters with `DS != 0` the drive number is written to the wrong address and the stage2 read uses garbage. Stage1 also never normalises `CS` with a far jump. |

### L20. `core::panic` / `core::panicf` are not declared noreturn

| Field | Value |
|-------|-------|
| **File** | `kernel/include/core/panic.hpp` |
| **Issue** | Both functions halt forever but lack `KERNEL_NORETURN`, so the compiler and cppcheck treat code after them as reachable (cppcheck `nullPointerRedundantCheck` in `kernel/sched/sched.cpp`, baselined in `config/cppcheck-suppressions.txt`). |
| **Fix** | Include `core/compiler.hpp` and declare both `KERNEL_NORETURN`; drop the suppression. |

### L21. `vfs_lseek` names locals `SEEK_SET`/`SEEK_CUR`/`SEEK_END`

| Field | Value |
|-------|-------|
| **File** | `kernel/fs/vfs.cpp:264` |
| **Issue** | These are libc macro names. Any header that defines them turns the declarations into `const int 0 = 0;`. cppcheck's std config does, so it reports `syntaxError` and **skips the whole file**; the suppression in `config/cppcheck-suppressions.txt` means `vfs.cpp` is currently not analysed. |
| **Fix** | Rename the locals (e.g. `WHENCE_SET`), or move named constants to `fs/vfs.hpp` and use them in `sys_execve.cpp` too; drop the suppression. |

---

## INFO — Known design decisions, planned features, or documentation gaps

### I2. Signals are default-action only and delivered cooperatively

`SYS_kill` is implemented, but only POSIX default actions: `Term`/`Core`
signals terminate the target, `Ign`/`Cont` are no-ops, and `Stop` signals
return `-ENOSYS` because there is no job control. Delivery is checked at
syscall exit and before a task blocks, so a CPU-bound task that never makes a
syscall does not notice a signal until it does. User handlers, `sigaction`,
`sigprocmask`, `sigpending` and `sigsuspend` remain `TODO.md` work. Only self
and direct children may be signalled; there are no UIDs yet.

### I3. Userland is a single standalone ring-3 image

Userland is built as standalone ELF images and entered at ring 3 through `int 0x80`. The first user task has a dedicated page directory with supervisor-only kernel mappings and user mappings for its image and stack. `fork` (copy-on-write), `execve` (static ELF32) and `waitpid` are implemented; there is no dynamic linker, no shared libraries, and no init/service supervision yet.

### I4. `config/` holds only the cppcheck suppressions

`config/cppcheck-suppressions.txt` exists for `scripts/cppcheck.sh`. The
Kconfig-based configuration system that `PROJECT_STRUCTURE.md` describes does
not exist yet; all kernel features are unconditionally compiled with hardcoded
defaults. Listed in `TODO.md`.

### I5. `tests/` directory is empty

All test infrastructure (unit tests for mm/sched/fs, integration tests, test runner) is documented in `TODO.md` as not-yet-implemented.

### I6. `tools/` subdirectories exist but are empty

`tools/build-iso/`, `tools/cross-compiler/`, `tools/mkinitrd/`, `tools/font-converter/` are empty directories. `tools/debug/` and `tools/kconfig/` are missing entirely. Documented in `TODO.md`.

### I7. `docs/`, `build/`, `iso/`, `boot/` directories don't exist but are documented

`PROJECT_STRUCTURE.md` describes these directories, but they have not been created yet.

### I9. Multiboot files are compiled but unused

`multiboot.asm` and `multiboot2.asm` are compiled and linked into the kernel, but the bootloader does not use Multiboot (it uses a custom stage2 instead). The headers exist for potential GRUB compatibility.

### I10. `TODO.md` has 50+ unchecked items

The TODO list documents extensive planned work including SMP, APIC, proper VFS, ext2, dynamic linker, pipe/signal infrastructure, etc.

---

## Quick Reference by Area

| Area | Critical | High | Medium | Low/Info |
|------|----------|------|--------|----------|
| **Bootloader (ASM)** | C9, C13 (unassembled) | H11, H20 | M20, M21, M22, M23, M24, M25, M26 | L14, L19 |
| **Bootloader (C++)** | — | H12, H18 | — | L5 |
| **Kernel (Core)** | — | — | M5, M6, M7, M8 | L12, L20 |
| **Kernel (Sched)** | — | — | M1, M41 | L13, I2 |
| **Kernel (FS)** | — | — | M4, M9, M31 | L12, L21 |
| **Kernel (MM)** | — | — | M10 | — |
| **Kernel (Dev)** | — | H13, H14 | M7, M10 | L8, L9 |
| **Kernel (Syscall)** | — | — | M1 | — |
| **Kernel (Arch)** | — | — | M1 | — |
| **Userland** | — | — | M17, M18, M19, M28, M35, M38 | L1, L2, L3, L4, L7, L17 |
| **Build System** | — | — | M13, M15 | L10, L16 |
| **Config/Tests/Tools** | — | — | — | I4, I5, I6, I7 |
| **Docs/Conventions** | — | — | M16 | I3, I9, I10 |

---

## Top 2 Most Important Fixes (remaining)

1. **M41 — orphan zombies while init blocks**: a hostile fork/exit loop
   accumulates zombies until the next prompt (needs SIGCHLD).
2. **M38 — `malloc()` size overflow**: an unchecked size can return a
   pointer with no space behind it and overrun the userland heap.
