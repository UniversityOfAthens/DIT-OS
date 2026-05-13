# DIT-OS Issue Tracker

This file catalogs all known issues across the codebase, organized by severity.
Generated from a comprehensive audit of kernel, bootloader, userland, build system, and configuration.

---

## CRITICAL — Will cause boot failure, crashes, or compile errors

### C5. No user pointer validation in syscall implementations

| Field | Value |
|-------|-------|
| **Files** | `kernel/syscall/sys_read.cpp`, `sys_write.cpp`, `sys_open.cpp`, `sys_getdents.cpp` |
| **Issue** | All syscall implementations directly dereference user-supplied buffer pointers without using `copy_from_user` / `copy_to_user`. A malicious user process can read/write arbitrary kernel memory. `sys_open.cpp` has a `copy_user_path()` helper that uses `volatile` but still directly dereferences without address-space validation. |
| **Violates** | AGENTS.md Security Rule 1: "Never dereference user-space pointers directly." |
| **Fix** | Implement `copy_from_user()` / `copy_to_user()` with address-space boundary checks and use them in all syscall handlers. |


### C9. PNG decoder header chunk lookup always fails

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/common/png.asm` |
| **Lines** | 38 (never-initialized `png_target_chunk`) |
| **Issue** | `png_target_chunk` is never initialized (remains 0), so `png_find_chunk` can never find any chunk. Additionally, `zlib_decompress` at line 191 is a stub that does nothing. PNG images are never decoded. |
| **Fix** | Implement proper chunk-type matching and a working zlib decompressor, or remove the dead code. |

### C10. CMake uses host compiler instead of cross-compiler

| Field | Value |
|-------|-------|
| **File** | `CMakeLists.txt` |
| **Lines** | 55, 83 |
| **Issue** | Uses bare `ld` and `g++` instead of `i686-elf-ld` / `i686-elf-g++`. Will fail on systems without multilib support or where `ld` defaults to 64-bit output. No `CROSS_COMPILE` or toolchain prefix variable. |
| **Fix** | Add a toolchain file or set the cross-compiler prefix. Use `i686-elf-g++`, `i686-elf-ld`, etc. |

### C11. `flake.nix` provides no cross-compiler

| Field | Value |
|-------|-------|
| **File** | `flake.nix` |
| **Issue** | Only native `gcc13` is provided. AGENTS.md mandates `i686-elf-gcc`/`i686-elf-g++` for kernel compilation. Building from the Nix environment requires a manually installed cross-compiler. |
| **Fix** | Add `pkgs.pkgsCross.i686-embedded.buildPackages.gcc13` or similar cross-compiler derivation to the flake. |

### C13. Disk LBA segment hardcoded to 0

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/common/disk.asm` |
| **Line** | 47 |
| **Issue** | `mov word [si+6], 0` always sets the buffer segment to 0 for LBA reads, ignoring any caller-specified segment. Data may be written to the wrong memory location if the caller expects a different segment. |
| **Fix** | Use the segment value passed by the caller, or document that the segment is always 0. |

---

## HIGH — Will cause build failures, incorrect behavior, or significant confusion

### H1. `.clang-format` enforces wrong brace style

| Field | Value |
|-------|-------|
| **File** | `.clang-format` |
| **Line** | 3 |
| **Issue** | `BasedOnStyle: LLVM` uses **attached braces** (`if () {`), but AGENTS.md mandates **Allman style** (brace on new line). Running `clang-format` will reformat all code to the wrong style. |
| **Fix** | Add `BreakBeforeBraces: Allman` override (or change to `BasedOnStyle: Linux` which uses Allman). |

### H2. `.clang-format` missing `UseTab: Never`

| Field | Value |
|-------|-------|
| **File** | `.clang-format` |
| **Line** | 10-11 |
| **Issue** | Comment says "UseTab: Never is often the default, but it's good to be explicit" but the actual `UseTab: Never` setting is absent. |
| **Fix** | Add `UseTab: Never` to the configuration. |

### H3. Missing referenced scripts

| Field | Value |
|-------|-------|
| **Scripts** | `scripts/check-toolchain.sh`, `scripts/run-qemu-debug.sh`, `scripts/gdb.sh` |
| **Issue** | These scripts are referenced by `AGENTS.md` and `.cursor/rules/` files but do not exist. The toolchain check, GDB debugging workflow, and QEMU debug mode are undocumented/inaccessible. |
| **Fix** | Create the missing scripts or update the documentation to remove references. |

### H4. `config/` directory completely empty

| Field | Value |
|-------|-------|
| **Directory** | `config/` |
| **Issue** | `PROJECT_STRUCTURE.md` defines 15+ expected files (Kconfig, kernel.config, Makefile.common, arch Kconfigs, etc.) but **none exist**. No configuration system exists — all kernel features are unconditionally compiled with hardcoded defaults. |
| **Fix** | Implement the Kconfig-based configuration system or remove references from documentation. |

### H8. Mutex struct not exported — dead code

| Field | Value |
|-------|-------|
| **File** | `kernel/sched/mutex.cpp:7-9` |
| **Issue** | The `Mutex` struct is defined only in the `.cpp` file (not in any header). No outside caller can create a `Mutex` to pass to `mutex_lock()` / `mutex_unlock()`. These functions are effectively dead code. |
| **Fix** | Move the `Mutex` struct definition to a header (e.g., `kernel/include/sched/mutex.hpp`). |

### H9. Stage2 wrong constants in `constants.inc`

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/common/constants.inc` |
| **Lines** | 6, 10 |
| **Issue** | `STAGE2_LOAD = 0x1000` (should be `0x7E00`), `STAGE2_SECTORS = 16` (should be `9`). These constants contradict the actual bootloader values in `boot.hpp` and `stage1.asm`. |
| **Fix** | Update to match `boot.hpp`: `STAGE2_LOAD = 0x7E00`, `STAGE2_SECTORS = 9`. |

### H10. Loader sector count mismatch (stage2 loads only 2 sectors)

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/stage2.asm:166` |
| **Issue** | Makefile declares `LOADER_SECTORS = 26`, but stage2 only loads 2 sectors (`mov word [dap_count], 2`). If loader.bin grows beyond 1 KB, it will be silently truncated. |
| **Fix** | Update `dap_count` to match `LOADER_SECTORS` or use a named constant. |

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

### H15. Busy-wait in serial read with `pause` instead of `hlt`/yield

| Field | Value |
|-------|-------|
| **File** | `kernel/syscall/sys_read.cpp` |
| **Line** | 27 |
| **Issue** | When no serial input is available, the loop uses `__asm__ volatile("pause")` which is a hint instruction that doesn't reduce power on all CPUs. Should use `sti; hlt` to sleep until the next interrupt, or yield to the scheduler. |
| **Fix** | Replace `pause` with `sti; hlt` or `sched_yield()`. |

### H16. Toolchain mismatch: documentation vs. reality

| Field | Value |
|-------|-------|
| **Polarity** | `AGENTS.md` documents `i686-elf-gcc`/`i686-elf-g++` as the toolchain, but `kernel/Makefile` and `CMakeLists.txt` use host `g++`/`gcc` with `-m32`. |
| **Issue** | Either the docs are wrong or the build system is wrong. On systems without 32-bit multilib support, the current build fails. |
| **Fix** | Standardize: either switch to `i686-elf-*` cross-compiler everywhere and update the build system, or update documentation to reflect the host-compiler approach. |

### H17. Userland compiled with `-ffreestanding` (contradicts guidelines)

| Field | Value |
|-------|-------|
| **File** | `kernel/Makefile` |
| **Line** | 81 |
| **Issue** | AGENTS.md says userland C should NOT be freestanding. But the kernel Makefile compiles userland sources (`init.c`, `shell/main.c`, libc files) with `-ffreestanding`. This prevents userland from using standard C library features properly. |
| **Fix** | Remove `-ffreestanding` from userland compilation flags. |

### H18. Dead C++ stage2 build path not invoked by any Makefile

| Field | Value |
|-------|-------|
| **Files** | All `.cpp` and `.hpp` files in `bootloader/src/stage2/` |
| **Issue** | `bootloader/src/stage2/Makefile` only builds the flat binary from `stage2.asm`. The C++ files (`gdt.cpp`, `paging.cpp`, `stage2.cpp`, etc.) are never compiled or linked. The `stage2.ld` linker script exists but is unused. This is misleading dead code. |
| **Fix** | Either wire up the ELF-linked C++ build path or remove the unused files. |

### H19. Loader sets EAX to multiboot magic (should not)

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/loader/loader.asm` |
| **Line** | 164 |
| **Issue** | `mov eax, MB_MAGIC` (0x2BADB002) sets EAX before jumping to the kernel. Per the Multiboot spec, the **bootloader receives** the magic value — it does not set it for the kernel. The kernel checks EAX on entry. Setting EAX to a fixed value defeats this check and would confuse a Multiboot-compliant kernel. |
| **Fix** | Remove the `mov eax, ...` instruction before the kernel jump (or ensure the kernel doesn't check EAX if not using Multiboot). |

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

### M2. `heap_free()` is a silent no-op

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/mm/heap.cpp` |
| **Lines** | 27-29 |
| **Issue** | The bump allocator's `heap_free()` accepts any pointer and silently does nothing. Any code that frees memory and expects reuse will silently leak. |
| **Fix** | At minimum, add a warning or assertion when `kfree` is called. Implement proper free once PMM/VMM is wired. |

### M3. Missing final newlines in ~15 files

| Field | Value |
|-------|-------|
| **Files** | `sys_getpid.cpp`, `sys_fork.cpp`, `pid.cpp`, `exit.cpp`, `fork.cpp`, `mutex.cpp`, `vga.hpp`, `color.hpp`, `serial.hpp`, `ata.hpp`, `floppy.hpp`, `pci_ids.hpp`, `ioctl.hpp`, `framebuffer.hpp`, `io.hpp` |
| **Issue** | POSIX requires files to end with a newline. GCC may warn with `-Wnewline-eof`. |
| **Fix** | Append newlines to all affected files. |

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

### M7. `syscall_table.cpp` mixes symbolic and hardcoded errno values

| Field | Value |
|-------|-------|
| **File** | `kernel/syscall/syscall_table.cpp`, `kernel/syscall/fd.cpp` |
| **Issue** | Some returns use `-EINVAL`, `-EBADF` correctly, but `syscall_table.cpp:23` uses `return -22; /* -EINVAL */` and `sys_fork.cpp:11` uses `return -38; /* -ENOSYS */`. Hardcoded values are fragile. |
| **Fix** | Replace all hardcoded negative numbers with symbolic `-ERRNO` constants. |

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

### M11. All shell scripts use `set -e` instead of `set -euo pipefail`

| Field | Value |
|-------|-------|
| **Files** | `scripts/build.sh`, `scripts/create-disk.sh`, `scripts/run-qemu.sh`, `bootloader/scripts/sign.sh`, `bootloader/scripts/build.sh` |
| **Issue** | AGENTS.md mandates `set -euo pipefail` for all shell scripts, but none use it. Without `-u`, undefined variables silently resolve to empty strings. Without `-o pipefail`, pipeline errors are silently ignored. |
| **Fix** | Add `set -euo pipefail` and `IFS=$'\n\t'` to all shell scripts. |

### M12. `scripts/create-disk.sh` hides `dd` errors

| Field | Value |
|-------|-------|
| **File** | `scripts/create-disk.sh` |
| **Lines** | 17-21 |
| **Issue** | All `dd` commands redirect stderr to `/dev/null`. If a disk write fails (e.g., out of space), the error is silently hidden and subsequent operations continue with incomplete data. |
| **Fix** | Remove `2>/dev/null` or add explicit error checking after each `dd`. |

### M13. `scripts/create-disk.sh` uses magic numbers for disk layout

| Field | Value |
|-------|-------|
| **File** | `scripts/create-disk.sh` |
| **Lines** | 16-21 |
| **Issue** | Sector counts and offsets (2880, 2, 11, 37) are hardcoded instead of using named constants. Must stay in sync with `bootloader/Makefile`, `constants.inc`, and `boot.hpp`. |
| **Fix** | Derive values from a shared configuration source (e.g., a `disk_layout.sh` that reads from `boot.hpp`). |

### M14. `.gitignore` does not exclude `kernel/build/`

| Field | Value |
|-------|-------|
| **File** | `.gitignore` |
| **Issue** | `build/` only matches the CMake build directory at root. `kernel/build/` (where the kernel Makefile puts object files) is not excluded. |
| **Fix** | Add `kernel/build/` to `.gitignore`. |

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
| **File** | `bootloader/src/common/png.asm` |
| **Lines** | 129, 136 |
| **Issue** | `movzx ecx, word [png_chunk_length]` truncates 32-bit chunk lengths to 16 bits. Chunks larger than 64 KB (IHDR, IDAT) will overflow and cause incorrect reads. |
| **Fix** | Use full 32-bit `mov ecx, [png_chunk_length]` instead of `movzx` from 16 bits. |

### M21. A20 gate uses only "fast method" — no fallback

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/common/a20.asm` |
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
| **File** | `bootloader/src/common/uefi.asm` |
| **Line** | 22 |
| **Issue** | COFF header says `dw 2` (2 sections), but there are 3 section headers (.text, .data, .reloc). A PE loader may skip the .reloc section, causing relocation failures. |
| **Fix** | Change `dw 2` to `dw 3` to match the actual number of sections. |

### M24. All UEFI PE section raw data sizes are 0

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/common/uefi.asm` |
| **Lines** | 84-106 |
| **Issue** | All section headers have `RawSize = 0` and `VirtualSize = 0`. No actual code or data is produced. The PE file is structurally incomplete. |
| **Fix** | Set proper sizes in the section headers. |

### M25. `memory.asm` C/ASM calling convention mismatch

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/common/memory.asm` (ASM) vs `bootloader/include/memory.hpp:25` (C++) |
| **Issue** | ASM function expects a FAR pointer (segment:offset) passed via registers `di` (offset) and `bx` (segment). The C declaration `int memory_map_e820(E820Entry *entries, int max_count)` passes a NEAR pointer. The calling conventions don't match. |
| **Fix** | Standardize on one calling convention (preferably NEAR with the flat binary's segment setup). |

### M26. `stage2_main.asm` loads kernel from wrong CHS address

| Field | Value |
|-------|-------|
| **File** | `bootloader/src/stage2/stage2_main.asm` |
| **Lines** | 217-219 |
| **Issue** | The ELF build path reads the kernel from CHS sector 6, head 1 (≈ LBA 23), but the Makefile places the kernel at LBA 37. The ELF-linked build path would read the wrong data. |
| **Fix** | Update CHS address or use LBA addressing consistent with the Makefile. |

### M27. Per-syscall handler null entries in dispatch table

| Field | Value |
|-------|-------|
| **File** | `kernel/syscall/syscall_table.cpp` |
| **Issue** | 10 syscalls (`stat`, `fstat`, `execve`, `waitpid`, `mmap`, `munmap`, `brk`, `kill`, `ioctl`) have no handler registered (returns `-ENOSYS`). `fork` returns `-ENOSYS` from a stub. The system cannot run multi-process userland. |
| **Fix** | Implement the syscalls or document as planned-but-not-implemented. |

### M28. `cmd_cat()` uses small 256-byte read buffer

| Field | Value |
|-------|-------|
| **File** | `userland/shell/main.c` |
| **Line** | 254 |
| **Issue** | `char chunk[256]` — adequate for ramfs but would cause many read syscalls for larger files. |
| **Fix** | Increase buffer size or make it a configurable constant. |

### M29. Stale build artifacts checked into repo

| Field | Value |
|-------|-------|
| **Directories** | `userland/libc/build/`, `userland/bin/build/` |
| **Issue** | Object files (`.o`), executables, and libraries (`.a`) are checked into the repository alongside orphaned source directories. Some `.o` files have no corresponding source. Build is not reproducible from the checked-in sources. |
| **Fix** | Remove build artifacts from version control. Add them to `.gitignore`. |

### M30. `linker.ld` uses `ENTRY(kernel_entry)` but multiboot header may conflict

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/boot/linker.ld` |
| **Issue** | The linker script specifies `ENTRY(kernel_entry)` but also links `multiboot.asm` and `multiboot2.asm` which place their own headers at the start of `.text`. Multiboot headers must be within the first 8192 bytes of the kernel image — the linker script order determines whether they are. |
| **Fix** | Verify and document the expected section ordering. |

### M31. Missing `KERNEL_PACKED` on ABI-boundary structs

| Field | Value |
|-------|-------|
| **Files** | `kernel/include/fs/dirent.hpp`, `kernel/include/fs/stat.hpp` |
| **Issue** | Structs that cross the user/kernel boundary (dirent, stat) lack `__attribute__((packed))` / `KERNEL_PACKED`. If compiler padding differs between kernel and userland, data will be misaligned. |
| **Fix** | Add `KERNEL_PACKED` to all ABI structs. |

### M32. Boot console prints appear before serial init completes

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/boot/main.cpp` |
| **Issue** | Early boot messages are printed to VGA before serial initialization is guaranteed to complete. On real hardware, serial may miss the first few messages. |
| **Fix** | Ensure serial is fully initialized before any console output, or buffer early messages. |

### M33. `entry.asm` does not clear .bss properly

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/boot/entry.asm` |
| **Issue** | The BSS-clearing logic may not handle all edge cases (e.g., BSS not 4-byte aligned, or zero-length BSS). |
| **Fix** | Review and harden the BSS clearing loop. |

### M34. `multiboot.asm` sets VBE mode `0x010F` without checking if VBE is available

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/boot/multiboot.asm` |
| **Line** | 196 |
| **Issue** | The multiboot header requests VBE mode `0x010F` unconditionally. If the bootloader (or BIOS, in the unlikely event this is used with GRUB) doesn't support VBE, this may cause the boot to fail or behave unexpectedly. |
| **Fix** | Make VBE mode conditional or document as dead code. |

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

### L6. Some files use `#pragma once` instead of include guards

| Field | Value |
|-------|-------|
| **Files** | Check `kernel/include/` for any use of `#pragma once`. AGENTS.md mandates `#ifndef`/`#define`/`#endif`. |

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
| **Issue** | Missing entries for `kernel/build/`, `*.swp`, `*.swo`, `*.swx` (vim swap), `*~`, `.directory`, `.DS_Store`, `Thumbs.db`, `.cache/`, `compile_commands.json`, `.vscode/`, `.idea/`. |

### L11. `entry.asm` uses hardcoded segment values

| Field | Value |
|-------|-------|
| **File** | `kernel/arch/x86_64/boot/entry.asm` |
| **Issue** | Segment selectors (e.g., `0x10` for data segment) are hardcoded instead of using named constants. |

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

### L15. `flake.nix` has stale nixpkgs pin (nixos-24.11 vs expected 25.05)

| Field | Value |
|-------|-------|
| **File** | `flake.nix:4` |
| **Issue** | Pinned to `nixos-24.11`. As of May 2026, `nixos-25.05` may be available. |

### L16. `flake.nix` missing formatter, checks, and devShell hook

| Field | Value |
|-------|-------|
| **File** | `flake.nix` |
| **Issue** | No `formatter` output (for `nix fmt`), no `checks` (for `nix flake check`), and no shell hook (for `nix develop` post-setup). |

### L17. No `ENOSYS` in userland errno

| Field | Value |
|-------|-------|
| **Files** | Userland `errno.h` (doesn't exist) |
| **Issue** | Until userland has an errno.h, users cannot programmatically check syscall errors. |

---

## INFO — Known design decisions, planned features, or documentation gaps

### I1. Two shell implementations coexist

Kernel has `kernel/sched/shell.cpp` (bootstrap stub) and userland has `userland/shell/main.c`. Both are compiled into the kernel. AGENTS.md explains this is intentional — the kernel stub is for early boot debugging, the userland shell is the real shell.

### I2. 10 syscalls defined but unimplemented

`SYS_stat`, `SYS_fstat`, `SYS_execve`, `SYS_waitpid`, `SYS_mmap`, `SYS_munmap`, `SYS_brk`, `SYS_kill`, `SYS_ioctl` are defined in the enum but have no handler (return `-ENOSYS`). Fork is a stub. Listed in `TODO.md` as planned.

### I3. Userland is compiled into the kernel (temporary)

Per AGENTS.md and Makefile comments, userland code is compiled and linked into the kernel binary for now. True userspace separation (ring 3, separate page tables) is not yet implemented.

### I4. `config/` directory and its 15+ files are documented but unimplemented

`PROJECT_STRUCTURE.md` describes a full Kconfig-based configuration system that does not exist yet. Listed in `TODO.md`.

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
| **Bootloader (ASM)** | C9, C13 | H9, H10, H11, H12, H19, H20 | M20, M21, M22, M23, M24, M25, M26 | L14 |
| **Bootloader (C++)** | — | H12 | — | L5 |
| **Kernel (Core)** | C5 | H15 | M2, M5, M6, M7, M8, M32, M34 | L6, L10, L12 |
| **Kernel (Sched)** | — | H8 | M1 | L13 |
| **Kernel (FS)** | — | — | M4, M9, M31 | L12 |
| **Kernel (MM)** | — | — | M2, M10 | — |
| **Kernel (Dev)** | — | H13, H14 | M10 | L8, L9 |
| **Kernel (Syscall)** | C5 | H15 | M1, M27 | — |
| **Kernel (Arch)** | — | H15 | M1, M33 | L11 |
| **Userland** | — | — | M17, M18, M19 | L1, L2, L3, L4, L7 |
| **Build System** | C10, C11 | H1, H2, H3, H16, H17 | M11, M12, M13, M14, M15, M30 | L15, L16 |
| **Config/Tests/Tools** | — | H4 | — | I4, I5, I6, I7 |
| **Docs/Conventions** | — | — | M16 | I1, I2, I3, I8, I9 |

---

## Top 5 Most Important Fixes (remaining)

1. **C5 — No user pointer validation**: All syscall handlers directly dereference user addresses without `copy_from_user`/`copy_to_user`. Security hole.
2. **C10, C11 — Cross-compiler**: Build system won't work on systems without 32-bit multilib support.
3. **C9 — PNG decoder is broken**: `zlib_decompress` stub, `png_target_chunk` never initialized (dead code path).
4. **H1, H2 — `.clang-format` wrong**: LLVM attached braces instead of Allman; missing `UseTab: Never`.
5. **H8 — Mutex not exported**: `Mutex` struct defined in `.cpp` only, `mutex_lock`/`unlock` are dead code.
