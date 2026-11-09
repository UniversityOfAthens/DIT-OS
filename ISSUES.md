# DIT-OS Issue Tracker

This file catalogs all known issues across the codebase, organized by severity.
Generated from a comprehensive audit of kernel, bootloader, userland, build system, and configuration.
Fixed issues are removed from this file; see `CHANGELOG.md` for what was fixed and when.

---

## CRITICAL — Will cause boot failure, crashes, or compile errors

None open.

---

## HIGH — Will cause build failures, incorrect behavior, or significant confusion

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

### M19. `_exit()` calls inline `exit()` — fragile coupling

| Field | Value |
|-------|-------|
| **File** | `userland/libc/src/stdlib/exit.c` |
| **Line** | 8 |
| **Issue** | `_exit(code)` calls `exit(code)`, which is a static inline function from `syscall.h`. If `syscall.h` is not included or the inline is removed, `_exit` silently breaks. |
| **Fix** | Have `_exit` call `dit_sys_exit()` directly instead of going through the inline wrapper. |

### M28. `cmd_cat()` uses small 256-byte read buffer

| Field | Value |
|-------|-------|
| **File** | `userland/shell/main.c` |
| **Line** | `copy_fd()` |
| **Issue** | `char chunk[256]` — adequate for ramfs but would cause many read syscalls for larger files. |
| **Fix** | Increase buffer size or make it a configurable constant. |

### M31. Missing `KERNEL_PACKED` on ABI-boundary structs

| Field | Value |
|-------|-------|
| **Files** | `kernel/include/fs/dirent.hpp`, `kernel/include/fs/stat.hpp` |
| **Issue** | Structs that cross the user/kernel boundary (dirent, stat) lack `__attribute__((packed))` / `KERNEL_PACKED`. If compiler padding differs between kernel and userland, data will be misaligned. |
| **Fix** | Add `KERNEL_PACKED` to all ABI structs. |

### M47. `open()` ignores its flags, so every file is writable

| Field | Value |
|-------|-------|
| **Files** | `kernel/syscall/sys_open.cpp`, `kernel/fs/vfs.cpp`, `kernel/fs/ramfs/ramfs.cpp` |
| **Issue** | `sys_open_impl()` passes flags 0 to `vfs_open()`, which ignores them, and `file_write()` checks only that the inode is a regular file. A process can therefore open any file and write it even though the API implies read-only access; userland can overwrite the seeded `/greeter` image. |
| **Fix** | Define `O_RDONLY`/`O_WRONLY`/`O_RDWR` (and `O_CREAT`/`O_TRUNC` when creation is wanted), pass the user's flags through, store them on `fs::file`, and reject a write (or read) that the open mode does not allow. Per-inode permissions wait for a UID model. |

---

## LOW — Cleanup, style, and minor issues

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

### I2. Scheduling is cooperative; signals act on the way back to user mode

There is no preemption: the timer only counts ticks, so a task runs until it
blocks or exits. Signals are delivered at syscall exit and at IRQ/exception
return to ring 3, which is what lets `^C`, `^Z` or `kill` stop a task that
spins in user mode, but other tasks still do not run while it spins. The
permission model has no UIDs: a process may signal itself and its
descendants. init (PID 1) discards signals it has no handler for.

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
| **Bootloader (ASM)** | — | — | — | L14, L19 |
| **Bootloader (C++)** | — | — | — | — |
| **Kernel (Core)** | — | — | M5, M6, M7, M8 | L12, L20 |
| **Kernel (Sched)** | — | — | M1 | L13, I2 |
| **Kernel (FS)** | — | — | M4, M9, M31, M47 | L12, L21 |
| **Kernel (MM)** | — | — | M10 | — |
| **Kernel (Dev)** | — | H13, H14 | M7, M10 | L8, L9 |
| **Kernel (Syscall)** | — | — | M1 | — |
| **Kernel (Arch)** | — | — | M1 | — |
| **Userland** | — | — | M19, M28 | L2, L3, L4, L7 |
| **Build System** | — | — | M13, M15 | L10, L16 |
| **Config/Tests/Tools** | — | — | — | I4, I5, I6, I7 |
| **Docs/Conventions** | — | — | M16 | I3, I9, I10 |

---

## Top 2 Most Important Fixes (remaining)

1. **M28 — `cmd_cat()` uses a 256-byte read buffer**: correct but many
   syscalls per file; size the buffer from the syscall bounce limit.
2. **M31/L7 — ABI struct layout is not enforced**: `dirent` and `stat`
   cross the user/kernel boundary without packing, and userland has no
   compile-time assertion that its copies match the kernel's.
