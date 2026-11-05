# DIT-OS Project TODO

**Status**: Active development. Build chain works. Many known bugs (see `ISSUES.md`).

---

## VERSIONING PLAN

Semantic Versioning, interpreted for the pre-1.0 alpha series:

- **patch** — a bug fix, a hardening change, or a small increment on the
  existing tier (`stat`/`fstat` + `kill`, M36, M39, M40, M37, M42,
  H9/H10).
- **minor** — a tier a user can actually feel: several syscalls *and* the
  userland pieces that use them. `0.1.0` is the first usable release
  (signal handlers, `pipe`, `dup2`, `errno.h`, job control), not merely the
  next batch of syscalls.
- **major** — the 1.0.0 milestone: stable ABI, or a breaking disk-format /
  syscall change. Before 1.0, breaking changes still use a minor bump.

| Logical step | Bump | Target version | Status |
|--------------|------|----------------|--------|
| `stat`/`fstat`, `kill` + default signal delivery, cppcheck/agent tooling | patch | `0.0.10-alpha` | released 2026-11-02 |
| M40 orphan reparenting + init reaper, M36 PID 1 exit panic, M39 COW-safe `copy_to_user` | patch | `0.0.11-alpha` | released 2026-11-02 |
| M37 string bounds, H9/H10 boot constants, M42 COW failure-counter corruption | patch | `0.0.12-alpha` | released 2026-11-03 |
| Full POSIX signals (handlers, masks, `sigaction`, `SIGCHLD`, job control) incl. M41 orphan reaping, `pipe()`/`dup2()` + userland `errno.h`, `chdir`/`getcwd`, M38 (**first usable release**) | minor | `0.1.0-alpha` | released 2026-11-03 |
| M46 pipe heap cap, L24 floppy read retry, L25 TTY line buffer | patch | `0.1.1-alpha` | in progress |
| H20 BIOS geometry probe, L23 SIGCHLD auto-reap + SA_NOCLDWAIT, L22 sessions (setsid/getsid + job-control boundaries) | patch | `0.1.2-alpha` | in progress |
| POSIX timers + shared memory | minor | `0.2.0-alpha` | planned |
| Stable ABI / breaking disk-format or syscall changes | major | `1.0.0` | future |

Each individual fix also carries its own `**Bump**` row in `ISSUES.md`.

---

## CRITICAL BUGS (Fix these first)

See `ISSUES.md` for full details. These are the blockers:

- [x] **Kernel truncated on boot** — stage2 now loads 240 sectors (`KERNEL_SECTORS`; the 0.1.0 kernel needs ~212)
- [x] **Task stack init order wrong** — reordered pushes to match context.asm pop order
- [x] **Duplicate `outb()`** — removed duplicate definition in `arch/io.h`
- [x] **UB in interrupt dispatch** — asm now passes regs pointer explicitly to C
- [x] **`disk.asm` retry count uses address as counter** — changed to `mov si, 3`
- [x] **E820 uninitialized EBX** — added `xor ebx, ebx` before loop
- [x] **`kernel_stack_top` treated as value, not address** — changed to array declaration
- [x] **User/kernel isolation** — standalone ring-3 user task, dedicated page directory, supervisor-only kernel mappings, and checked user access
- [x] **CMake uses host compiler** — CMake now wraps the Makefiles, which pick `i686-elf-*` when available
- [x] **flake.nix has no cross-compiler** — `nix develop .#cross` adds it; default shell fixed (hardening off, no SSE)

---

---

## BOOTLOADER

### Stage1 (working — no changes needed)
- [x] `bootloader/src/stage1/stage1.asm` — 512-byte MBR
- [x] `bootloader/src/stage1/boot.hpp`
- [x] `bootloader/src/stage1/Makefile`

### Stage2 — flat binary build (ACTIVE)
- [x] `bootloader/src/stage2/stage2.asm` — Real mode entry, menu, kernel load
- [x] `bootloader/src/stage2/a20.asm` — Fast A20 (port 0x92 only — missing keyboard/BIOS fallback)
- [x] `bootloader/src/stage2/disk.asm` — INT 13h CHS + LBA (has retry counter bug)
- [x] `bootloader/src/stage2/fat32.asm` — FAT32 reader (has inverted branch bug)
- [x] `bootloader/src/stage2/font.asm`
- [x] `bootloader/src/stage2/gdt.asm`
- [x] `bootloader/src/stage2/graphics.asm` — VESA/VBE modes
- [x] `bootloader/src/stage2/memory.asm` — E820 (has uninitialized EBX bug)
- [x] `bootloader/src/stage2/menu.asm`
- [x] `bootloader/src/stage2/mouse.asm`
- [x] `bootloader/src/stage2/multiboot.asm`
- [x] `bootloader/src/stage2/pf2.asm`
- [x] `bootloader/src/stage2/pmode.asm`
- [x] `bootloader/src/stage2/png.asm` — STUB (zlib does nothing)
- [x] `bootloader/src/stage2/bmp.asm`
- [x] `bootloader/src/stage2/theme.asm`
- [x] `bootloader/src/stage2/video.asm`
- [x] `bootloader/src/stage2/stage2_core.asm`
- [x] `bootloader/src/stage2/stage2_main.asm` — Alternate entry
- [x] `bootloader/src/stage2/long_mode.asm` — Unused (32-bit only)
- [x] `bootloader/src/stage2/Makefile`

### Stage2 — C++ files (DEAD CODE, not compiled by any Makefile)
Marked as `[x]` because files exist, but **none are compiled into the flat binary**.

- [x] `bootloader/src/stage2/stage2.cpp`
- [x] `bootloader/src/stage2/console.cpp`
- [x] `bootloader/src/stage2/disk.cpp`
- [x] `bootloader/src/stage2/elf.cpp`
- [x] `bootloader/src/stage2/ext2.cpp`
- [x] `bootloader/src/stage2/fat.cpp`
- [x] `bootloader/src/stage2/fs.cpp`
- [x] `bootloader/src/stage2/gdt.cpp` — Broken include path
- [x] `bootloader/src/stage2/iso9660.cpp`
- [x] `bootloader/src/stage2/memory.cpp`
- [x] `bootloader/src/stage2/paging.cpp` — Broken include path
- [x] `bootloader/src/stage2/video.cpp`

### Stage2 improvements needed
- [ ] Wire up ELF-linked C++ build path OR remove dead C++ files
- [ ] Fix broken include paths in `gdt.cpp`, `paging.cpp`
- [ ] Fix 20+ duplicate global symbols between `stage2_core.asm` and module files (for ELF build)
- [ ] Add keyboard controller + BIOS fallback to A20 enable
- [ ] Implement proper PNG zlib decompression (ISSUES.md M43)
- [ ] Add LBA fallback (detect geometry dynamically instead of hardcoded 18/2)

### Loader
- [x] `bootloader/src/loader/loader.asm` — Protected mode entry, ELF parsing, kernel jump
- [x] `bootloader/src/loader/Makefile`

### Bootloader shared
- [x] `bootloader/include/boot.hpp`
- [x] `bootloader/include/config.hpp`
- [x] `bootloader/include/disk.hpp`
- [x] `bootloader/include/elf.hpp`
- [x] `bootloader/include/fs.hpp`
- [x] `bootloader/include/memory.hpp`
- [x] `bootloader/include/types.hpp`
- [x] `bootloader/include/video.hpp`
- [x] `bootloader/config/boot.cfg`
- [x] `bootloader/scripts/build.sh`
- [x] `bootloader/scripts/sign.sh`
- [x] `bootloader/Makefile`
- [x] `bootloader/src/common/constants.inc`
- [x] `bootloader/src/common/macros.inc`
- [x] Fix `constants.inc` — `STAGE2_LOAD` is now `0x7E00` and stage1 uses
      it; `STAGE2_SECTORS` is `9` and stage2 loads the full
      `LOADER_SECTORS` region with a Makefile size check (H9, H10)

### UEFI bootloader
- [x] `bootloader/src/uefi/uefi.asm` — STUB (section count mismatch, raw sizes = 0)

### Visual bootloader (custom, no GRUB)

Plan: make our own graphical boot experience. The stage1 → stage2 → loader
path stays the only boot path. **Bump**: minor (new user-visible feature);
natural home is with or after the `0.1.0-alpha` usable milestone.

- [ ] VBE mode picker with a fallback chain (native → 1024x768 → 640x480 → VGA text)
- [ ] Theme file (extend `bootloader/config/boot.cfg`): background, colours,
      logo, font, timeout, default entry
- [ ] Per-entry icons, kernel path, cmdline and theme override
- [ ] Countdown bar any key interrupts; arrows/Enter/Esc/number keys; serial
      VT100 parity with the current text menu
- [ ] Splash screen + boot progress, mouse pointer (stage2 has `mouse.asm`)
- [ ] Recovery / safe-mode entry that passes a flag on the kernel cmdline
- [ ] Strict theme parsing: reject a malformed config loudly instead of
      half-drawing (config parsing is in the CVE catalog)

---

## KERNEL

### Architecture — x86_64 boot
- [x] `kernel/arch/x86_64/boot/entry.asm` — kernel entry, BSS clear
- [x] `kernel/arch/x86_64/boot/main.cpp` — kernel_early_init
- [x] `kernel/arch/x86_64/boot/init.cpp` — kernel_init (subsystem init)
- [x] `kernel/arch/x86_64/boot/multiboot.asm` — Multiboot1 header
- [x] `kernel/arch/x86_64/boot/multiboot2.asm` — Multiboot2 header
- [x] `kernel/arch/x86_64/boot/trampoline.asm` — SMP trampoline
- [x] `kernel/arch/x86_64/boot/linker.ld`
- [x] `kernel/arch/x86_64/boot/Makefile`

### Architecture — CPU
- [x] `kernel/arch/x86_64/cpu/gdt.cpp/asm`
- [x] `kernel/arch/x86_64/cpu/idt.cpp/asm` — has UB in interrupt_dispatch
- [x] `kernel/arch/x86_64/cpu/tss.cpp/asm`
- [x] `kernel/arch/x86_64/cpu/pic.cpp/asm`
- [x] `kernel/arch/x86_64/cpu/apic.cpp` — disabled/stub
- [x] `kernel/arch/x86_64/cpu/lapic.cpp` — disabled/stub
- [x] `kernel/arch/x86_64/cpu/ioapic.cpp` — disabled/stub
- [x] `kernel/arch/x86_64/cpu/smp.cpp/asm` — disabled/stub
- [x] `kernel/arch/x86_64/cpu/msr.cpp/asm`
- [x] `kernel/arch/x86_64/cpu/cpuid.cpp/asm`
- [x] `kernel/arch/x86_64/cpu/control.cpp/asm`
- [x] `kernel/arch/x86_64/cpu/mtrr.cpp`
- [x] `kernel/arch/x86_64/cpu/pat.cpp`
- [x] `kernel/arch/x86_64/cpu/cpufeatures.cpp`
- [x] `kernel/arch/x86_64/cpu/arch.cpp`

### Architecture — Memory Management
- [x] `kernel/arch/x86_64/mm/pmm.cpp` — Simple bitmap PMM
- [x] `kernel/arch/x86_64/mm/vmm.cpp` — Stub/basic
- [x] `kernel/arch/x86_64/mm/paging.cpp/asm` — Page table helpers
- [x] `kernel/arch/x86_64/mm/heap.cpp` — Bump allocator (kfree is no-op)
- [x] `kernel/arch/x86_64/mm/early_mm.cpp`
- [x] `kernel/arch/x86_64/mm/memblock.cpp`
- [x] `kernel/arch/x86_64/mm/slab.cpp` — Stub
- [x] `kernel/arch/x86_64/mm/dma.cpp` — Stub
- [x] `kernel/arch/x86_64/mm/tlb.asm`
- [x] `kernel/arch/x86_64/mm/mtrr.asm`
- [x] Implement proper VMM with page fault handling (demand paging, copy-on-write, `mm/vmm.cpp`)
- [ ] Implement proper slab/buddy allocator (kernel heap is first-fit; user frames are a refcounted pool)
- [ ] Grow the user frame pool: it is 2 MiB below 4 MiB (`VMM_FRAME_START`..`VMM_FRAME_END`), so user
      space is capped at 512 pages and `pmm.cpp` is still unused (it would hand out frame 0)
- [x] Implement `copy_from_user` / `copy_to_user` with address-space checks (`kernel/arch/x86_64/mm/uaccess.cpp`)
- [x] Make `heap_free` actually free memory (first-fit with split and coalesce, `mm/heap.cpp`)

### Architecture — Device Drivers
- [x] `kernel/arch/x86_64/dev/serial.cpp` — COM1-4
- [x] `kernel/arch/x86_64/dev/keyboard.cpp` — PS/2 keyboard
- [x] `kernel/arch/x86_64/dev/mouse.cpp` — PS/2 mouse
- [x] `kernel/arch/x86_64/dev/timer.cpp` — PIT timer
- [x] `kernel/arch/x86_64/dev/rtc.cpp` — CMOS RTC
- [x] `kernel/arch/x86_64/dev/pci.cpp` — PCI enumeration
- [x] `kernel/arch/x86_64/dev/ata.cpp` — ATA PIO
- [x] `kernel/arch/x86_64/dev/ahci.cpp` — STUB (uses port I/O instead of MMIO)
- [x] `kernel/arch/x86_64/dev/nvme.cpp` — STUB
- [x] `kernel/arch/x86_64/dev/floppy.cpp` — STUB
- [ ] Fix AHCI to use MMIO instead of port I/O
- [ ] Fix ATA sector count bound check (num > 255)
- [ ] Implement USB subsystem (UHCI/OHCI/EHCI/XHCI)
- [ ] Implement NIC drivers (e1000, rtl8139, virtio-net)

### Architecture — Graphics
- [x] `kernel/arch/x86_64/graphics/vga.cpp` — VGA text mode
- [x] `kernel/arch/x86_64/graphics/framebuffer.cpp` — Basic framebuffer
- [ ] Implement VBE/VESA modes
- [ ] Add font subsystem

### Core
- [x] `kernel/core/kernel.cpp`
- [x] `kernel/core/panic.cpp`
- [x] `kernel/core/printk.cpp` — Duplicates VGA code with console.cpp
- [x] `kernel/core/console.cpp`
- [x] `kernel/core/kassert.cpp`
- [x] `kernel/core/version.cpp`
- [x] `kernel/core/config.cpp` — NO-OP, doesn't actually initialize config
- [x] `kernel/core/cmdline.cpp`
- [x] `kernel/core/logo.cpp`
- [x] `kernel/core/modules.cpp` — Stub
- [x] `kernel/core/symbols.cpp` — Stub
- [x] `kernel/core/Makefile`
- [ ] Deduplicate VGA output code between printk.cpp and console.cpp
- [ ] Implement real config_init from command line

### Filesystem
- [x] `kernel/fs/vfs.cpp` — Path resolution, open/read/write/readdir
- [x] `kernel/fs/ramfs/ramfs.cpp` — In-memory root filesystem
- [x] `kernel/fs/ramfs/ramfs.hpp`
- [x] `kernel/fs/ramfs/inode.cpp`
- [x] `kernel/fs/ramfs/Makefile`
- [ ] Implement buffer cache (`kernel/fs/buffer.cpp`)
- [ ] Implement proper path resolution in `namei.cpp`
- [ ] Add ext2 filesystem
- [ ] Add FAT32 filesystem
- [ ] Add devfs (/dev/null, /dev/zero, etc.)
- [ ] Add procfs

### Scheduler
- [x] `kernel/sched/sched.cpp` — Round-robin
- [x] `kernel/sched/task.cpp` — Task creation (stack init order WRONG)
- [x] `kernel/sched/context.asm` — Context switch
- [x] `kernel/sched/pid.cpp` — PID allocation
- [x] `kernel/sched/idle.cpp`
- [x] `kernel/sched/init_task.cpp`
- [x] `kernel/sched/shell.cpp` — Bootstrap serial shell
- [x] `kernel/sched/exit.cpp`
- [x] `kernel/sched/fork.cpp` — STUB (returns -ENOSYS)
- [x] `kernel/sched/mutex.cpp` — Struct not exported, dead code
- [x] Implement fork() properly
- [x] Implement execve() (static ELF32, `syscall/sys_execve.cpp`)
- [x] Implement waitpid() (with `WNOHANG`)
- [ ] Export Mutex struct to a header
- [x] Implement signal handling (default actions, cooperative delivery)
- [x] Signal handlers: `sigaction`, `sigprocmask`, `sigpending`, `sigsuspend`, `sigreturn`, SA_RESTART
- [x] Faults raise real signals (SIGSEGV/SIGILL/SIGFPE/SIGBUS/SIGTRAP)
- [x] Job control: process groups, stop/continue, `waitpid` WUNTRACED/WCONTINUED, console foreground group, ^C/^Z/^\
- [x] `SIGCHLD` to the parent, and to init for reparented zombies (M41)
- [ ] Preemptive scheduling (timer-driven; see `ISSUES.md` I2)
- [ ] Sessions (`setsid`) and orphaned process groups (`ISSUES.md` L22)
- [x] Reparent children to PID 1 when a task exits (M40)
- [x] Panic when PID 1 exits instead of idling forever (M36)

### Syscalls
- [x] `kernel/syscall/syscall.cpp` — Dispatcher
- [x] `kernel/syscall/syscall_table.cpp`
- [x] `kernel/syscall/syscall_trace.cpp` — No-op stub
- [x] `kernel/syscall/syscall_wrappers.cpp`
- [x] `kernel/syscall/userland_syscall_cwrap.cpp` — No pointer validation
- [x] `kernel/syscall/fd.cpp`
- [x] `kernel/syscall/sys_read.cpp`
- [x] `kernel/syscall/sys_write.cpp`
- [x] `kernel/syscall/sys_open.cpp`
- [x] `kernel/syscall/sys_close.cpp`
- [x] `kernel/syscall/sys_exit.cpp`
- [x] `kernel/syscall/sys_getpid.cpp`
- [x] `kernel/syscall/sys_getdents.cpp`
- [x] `kernel/syscall/sys_fork.cpp` — STUB (returns -ENOSYS)
- [x] Implement: execve, waitpid, mmap, munmap, brk
- [x] Implement: stat, fstat
- [x] Implement: kill (process or group)
- [x] Implement: pipe, dup, dup2, chdir, getcwd, setpgid, getpgid
- [x] Add `copy_from_user` / `copy_to_user` to all syscall handlers

### Kernel include headers
All kernel include headers exist (~80 files). See `PROJECT_STRUCTURE.md` for the full list.

- [x] All headers in `kernel/include/` exist
- [ ] Remove dead headers: `kernel/include/kernel.h`, `kernel/include/arch/io.h` (duplicate outb)
- [ ] Fix type system split: decide between `kernel::u8` (types.hpp) and global `u8` (types.h)
- [ ] Remove dead `kernel/include/arch/serial.h` (conflicts with kernel.h)
- [ ] Add missing config headers: `kernel/include/config/{debug,mm,sched,fs}.hpp`
- [ ] Add `ENOSYS` to `errno_defs.hpp`

### Library (kernel/lib/)
- [ ] Implement string ops (memcpy, memset, strlen, strcmp)
- [ ] Implement basic data structures (list, queue, bitmap, rbtree)
- [ ] All files in `kernel/lib/` are not yet created

### Dead/legacy files (to remove)
- [ ] `kernel/main.cpp` — Old kernel_main, not compiled
- [ ] `kernel/kernel.asm` — Old 16-bit entry, not compiled
- [ ] `kernel/arch/x86_64/serial.cpp` — C-style serial, conflicts with dev/serial.cpp
- [ ] `kernel/arch/x86_64/printk.cpp` — C-style printk, conflicts with core/printk.cpp

---

## USERLAND

### Headers
- [x] `userland/include/syscall.h` — Syscall numbers + inline wrappers
- [x] `userland/libc/include/stdio.h`
- [x] `userland/libc/include/stdlib.h`
- [x] Add `errno.h` (constants; wrappers return `-errno`)
- [x] Add `signal.h` (`sigaction`, masks, `raise`, `pause`)
- [ ] Add an `errno` variable (`ISSUES.md` M17)
- [x] Add `sys/stat.h` (`struct stat`, `stat()`/`fstat()`)
- [ ] Add `string.h`, `unistd.h`, `fcntl.h`, `sys/types.h`

### libc sources
- [x] `userland/libc/src/stdio/printf.c`
- [x] `userland/libc/src/stdlib/malloc.c`
- [x] `userland/libc/src/stdlib/exit.c`
- [ ] Implement free()
- [ ] Implement string ops (memcpy, memset, strlen, strcmp)
- [ ] Implement read/write wrappers
- [ ] Point userland `malloc()` at `brk()` instead of its static 64 KiB buffer

### Runtime
- [x] `userland/crt0.asm` — clears BSS, receives argc/argv, calls `main`, then exits
- [x] Standalone userland ELF build linked at `0x00400000`

### Binaries
- [x] `userland/bin/init.c` — Init process
- [ ] `userland/bin/ls.c` (stale .o exists, no source)
- [ ] `userland/bin/cat.c` (stale .o exists, no source)
- [ ] Add echo, mkdir, cp, mv, rm, pwd, etc.

### Shell
- [x] `userland/shell/main.c` — Job-control shell: programs, `|` pipelines, `&`, `jobs`/`fg`/`bg`, builtins `cat`, `cd`, `echo`, `exit`, `export`, `ls`, `pwd`
- [ ] Shell: redirections (`<`, `>`), `$?`, a `PATH` search
- [ ] Fix silent error swallowing (getdents/read errors treated as EOF)
- [ ] Add proper command parser
- [ ] Add I/O redirection support

---

## BUILD SYSTEM

### Makefiles
- [x] `kernel/Makefile` — Builds kernel ELF + compiles userland sources
- [x] `bootloader/Makefile` — Assembles disk.img, runs QEMU
- [x] All sub-Makefiles for stage1, stage2, loader, core, ramfs
- [x] Standardize on cross-compiler (i686-elf-*) or document host-gcc approach — auto-detected, `CROSS_COMPILE` override
- [x] Kernel build uses `-O2 -Werror -MMD -MP -march=i686 -mgeneral-regs-only`

### CMake
- [x] `CMakeLists.txt` — thin wrapper over the Makefiles (targets: kernel, disk, run, debug, os-clean)

### Nix flake
- [x] `flake.nix` + `flake.lock`
- [x] Add `i686-elf-gcc` cross-compiler package (`devShells.cross`)
- [ ] Add formatter and check outputs

### Scripts
- [x] `scripts/build.sh`
- [x] `scripts/create-disk.sh`
- [x] `scripts/qemu-test.sh` — QEMU integration test (boot, self-test, shell commands, tab completion, cd/pwd, programs, pipelines, ^C/^Z job control, stress, poweroff)
- [x] `scripts/run-qemu.sh`
- [ ] `scripts/check-toolchain.sh` — Referenced in docs but doesn't exist
- [ ] `scripts/run-qemu-debug.sh`
- [ ] `scripts/gdb.sh`
- [x] Add `set -euo pipefail` to all scripts (enforced by the lint workflow)
- [x] Add `readonly` for constants
- [x] Fix create-disk.sh to not hide dd errors

---

## INFRASTRUCTURE

### GitHub / CI/CD
- [x] `.github/workflows/ci.yml` — build, layout/SSE checks, QEMU integration test, Nix shell, CMake wrapper
- [x] `.github/workflows/lint.yml` — clang-format (pinned), shellcheck, source conventions, actionlint
- [x] `.github/workflows/release.yml` — tag-driven, boot-tested, notes from CHANGELOG.md
- [x] `.github/workflows/hello.yaml`
- [x] `.github/ISSUE_TEMPLATE/bug_report.md`
- [x] `.github/ISSUE_TEMPLATE/feature_request.md`
- [x] `.github/PULL_REQUEST_TEMPLATE.md`

### IDE support
- [ ] `.vscode/settings.json`
- [ ] `.vscode/tasks.json`
- [ ] `.vscode/launch.json`

### Config system
- [ ] Kconfig system (`config/` currently holds only `cppcheck-suppressions.txt`)

### Tests
- [ ] All files in `tests/` — directory is empty, no tests exist

### ISO / initrd
- [ ] All files in `iso/` — directory is empty
- [ ] All files in `initrd/` — directory is empty

### Documentation
- [ ] All files in `docs/` — directory is empty

### Root files
- [ ] `Kbuild`
- [ ] `Kconfig`
- [ ] `CONTRIBUTING.md`
- [ ] `.editorconfig`
- [ ] `.gitattributes`
- [ ] `Doxyfile`

---

## KNOWN ISSUES (See `ISSUES.md` for full catalog)

The file `ISSUES.md` contains a comprehensive audit with 13 critical, ~25 high, ~30 medium, and ~15 low-priority issues. Key categories:

- **Bootloader**: Kernel truncation, FAT32 inverted branch, E820 uninitialized EBX, disk.asm retry counter, PNG zlib stub, UEFI PE header broken
- **Kernel**: Task stack init order wrong, duplicate outb(), UB in interrupt dispatch, kernel_stack_top as value not address, heap_free no-op, Mutex not exported, conflicting serial_init declarations
- **Syscall**: every syscall in `syscall_numbers.hpp` has a handler, including signal handlers, job control, pipes and the working directory. User pointers are validated through `mm::copy_*_user` / `mm::user_range_ok`
- **Build system**: CMake uses host compiler, flake.nix missing cross-compiler, .clang-format wrong brace style, shell scripts missing set -euo pipefail, missing scripts referenced in docs
- **Userland**: no `errno` variable, error returns silently swallowed, _exit() calls inline exit()

---

## PRODUCTION-LEVEL ROADMAP

### Phase 1: Stabilize (current bugs — priority)
- [x] Add `copy_from_user`/`copy_to_user` for syscall security
- [ ] Fix heap_free to actually free memory (slab/buddy allocator)
- [ ] Implement proper VMM with page fault handling and demand paging
- [x] Standardize cross-compiler toolchain (i686-elf-*) in all build paths
- [x] Fix shell scripts with `set -euo pipefail`
- [ ] Export Mutex struct to header
- [ ] Fix `.clang-format` and run as part of CI

### Phase 2: POSIX Core — Process model & syscalls
- [x] Implement fork() with proper address space cloning (copy-on-write)
- [x] Implement execve() with ELF loader for user binaries (static ELF32 only)
- [x] Implement waitpid() for process lifecycle
- [x] Implement brk()/mmap()/munmap() for heap management (anonymous memory only)
- [x] Implement stat()/fstat() for file metadata
- [x] Implement kill() and signal delivery (default actions, cooperative)
- [x] Implement ioctl() for device control (fds 0-2: console modes and power off)
- [x] Implement pipe() for inter-process communication (anonymous pipes)
- [ ] Named FIFOs (`mkfifo`)
- [x] Full POSIX signal handling (sigaction, sigpending, sigsuspend)
- [x] dup()/dup2() and fd 0-2 as ordinary files
- [x] chdir()/getcwd() with per-process working directory
- [ ] POSIX timers (setitimer, nanosleep)
- [ ] Shared memory (shmget, shmat, shmdt)

### Phase 3: Memory Management
- [ ] Slab allocator for small kernel objects
- [ ] Buddy allocator for physical page management
- [x] Demand paging with page fault handler
- [ ] Swap space and swap file support
- [ ] Memory-mapped files (mmap of regular files)
- [ ] mprotect() for per-page permissions
- [ ] mlock/munlock for wired pages
- [ ] NUMA-aware allocations
- [ ] Per-CPU page allocator
- [ ] Huge pages (2MiB / 1GiB)
- [ ] OOM killer
- [ ] /proc/meminfo, /proc/pagetypeinfo

### Phase 4: Storage & Filesystems
- [ ] Buffer cache layer
- [ ] Block device abstraction (request queue, I/O scheduler)
- [ ] Deadline / CFQ / noop I/O schedulers
- [ ] ext2 filesystem (read+write)
- [ ] FAT32 filesystem (read+write)
- [ ] ISO9660 filesystem (read-only)
- [ ] tmpfs
- [ ] devfs (/dev/null, /dev/zero, /dev/random, /dev/tty, etc.)
- [ ] procfs (/proc/self, /proc/cpuinfo, /proc/meminfo, /proc/pid/)
- [ ] sysfs device model
- [ ] Filesystem journaling (ext3-style)
- [ ] Disk encryption (dm-crypt / LUKS-style)
- [ ] Logical Volume Manager (LVM-style)
- [ ] RAID (mdadm-style — 0, 1, 5, 6, 10)
- [ ] Mount namespaces
- [ ] Quotas

### Phase 5: Userland Environment
- [x] User/kernel privilege separation (ring 3)
- [x] Separate page tables per process
- [ ] Full POSIX libc (stdio, stdlib, string, unistd, signal, time, math, locale)
- [ ] Dynamic linker (ld.so) for shared libraries
- [x] crt0 with BSS clear, argc/argv, proper exit
- [ ] Init system (PID 1 that reaps orphans)
- [ ] Service manager (launch daemons, dependencies, supervision)
- [ ] Login manager (getty, login, session management)
- [ ] Device manager (udev-style — hotplug, dev node creation)
- [ ] D-Bus for inter-process communication
- [ ] System logging (syslog / journald-style)

### Phase 6: Security & Isolation
- [ ] ASLR (kernel + user space)
- [ ] KASLR (kernel base address randomization)
- [ ] Stack canaries (compiler -fstack-protector)
- [ ] SMEP/SMAP (supervisor-mode execution prevention / access prevention)
- [ ] NX bit (non-executable stack and pages)
- [ ] IOMMU (VT-d) for device isolation
- [ ] Secure Boot + UEFI verification
- [ ] Signed kernel modules
- [ ] SELinux / AppArmor LSM-style mandatory access control
- [ ] Control Flow Integrity (CFI) / IBT
- [ ] seccomp for syscall filtering
- [ ] Namespaces (PID, mount, network, user, cgroup)
- [ ] cgroups v2 for resource limits (CPU, memory, I/O)
- [ ] Capabilities (POSIX 1e capabilities)
- [ ] Encrypted swap
- [ ] Kernel lockdown (lockdown=confidentiality/integrity)

### Phase 7: Networking Stack
- [ ] Network interface abstraction layer
- [ ] Ethernet driver framework
- [ ] Loopback device
- [ ] ARP protocol
- [ ] IPv4 stack (routing, fragmentation, options)
- [ ] ICMP (ping, error reporting)
- [ ] UDP (sockets, sendto/recvfrom)
- [ ] TCP (sockets, connect/accept, flow control, congestion control)
- [ ] BSD socket API (socket, bind, listen, accept, connect)
- [ ] DNS resolver (stub resolver)
- [ ] DHCP client
- [ ] NTP client
- [ ] Firewall (netfilter-style packet filtering)
- [ ] Network bridge / VLAN
- [ ] WireGuard VPN
- [ ] TLS (mbedTLS / BearSSL-style)
- [ ] NIC drivers (e1000, rtl8139, virtio-net)

### Phase 8: Device Drivers
- [ ] AHCI/SATA (proper MMIO, not port I/O)
- [ ] NVMe driver (proper)
- [ ] USB 2.0 (EHCI) + USB 3.0 (xHCI)
- [ ] USB HUB driver
- [ ] USB HID (keyboard, mouse, gamepad)
- [ ] USB mass storage
- [ ] GPU — VBE framebuffer (vesafb)
- [ ] GPU — simple DRM/KMS driver (bochs, virtio-gpu, vmware-svga)
- [ ] GPU — 2D acceleration, modesetting
- [ ] Audio (AC97, HDA, SoundBlaster 16)
- [ ] ATA PIO improvements (error handling, DMA)
- [ ] Floppy driver cleanup
- [ ] PS/2 mouse (full protocol)
- [ ] RTC alarm support
- [ ] ACPI (AML interpreter, power management, sleep states)
- [ ] SMBIOS / DMI reading

### Phase 9: Multi-Processor & Concurrency
- [ ] SMP boot (APIC, SIPI, trampoline)
- [ ] Local APIC timer (per-CPU timer)
- [ ] I/O APIC interrupt routing
- [ ] Per-CPU run queues
- [ ] Spinlocks with proper memory ordering
- [ ] Reader-writer locks
- [ ] RCU (Read-Copy-Update)
- [ ] Lock-free data structures (lockless lists, queues)
- [ ] CPU hotplug
- [ ] Scheduler — CFS (Completely Fair Scheduler)
- [ ] Scheduler — real-time priorities (FIFO, RR)
- [ ] Scheduler — load balancing across CPUs
- [ ] Scheduler — deadline scheduler
- [ ] CPU frequency scaling (cpufreq)
- [ ] CPU idle states (cpuidle)

### Phase 10: Reliability & Debugging
- [ ] Kernel crash dumps (kdump-style)
- [ ] Stack unwinder (DWARF / frame pointer based)
- [ ] GDB stub (kgdb-style)
- [ ] ftrace / tracepoints
- [ ] eBPF bytecode verifier + runtime
- [ ] perf — performance counters
- [ ] Error Decode (EDAC) for memory errors
- [ ] Watchdog timer (hardware + software)
- [ ] Filesystem consistency checker (fsck)
- [ ] Journal recovery
- [ ] Liveness / hung task detection
- [ ] Lockdep-style deadlock detection
- [ ] kmemleak (kernel memory leak detector)
- [ ] KASAN (kernel address sanitizer)
- [ ] UBSAN (undefined behavior sanitizer)

### Phase 11: Virtualization
- [ ] KVM hypervisor support (KVM-on-xxx)
- [ ] VirtIO — block device
- [ ] VirtIO — network device
- [ ] VirtIO — GPU
- [ ] VirtIO — input (keyboard/mouse)
- [ ] VirtIO — balloon (memory overcommit)
- [ ] Para-virtualized clock
- [ ] Hyper-V enlightenments
- [ ] VMware backdoor interface

### Phase 12: Performance Optimization
- [ ] Multi-queue block I/O (blk-mq)
- [ ] Per-CPU page cache
- [ ] Read-ahead for sequential reads
- [ ] Writeback throttling
- [ ] Large folios for page cache
- [ ] Zero-copy networking (sendfile, splice)
- [ ] TCP segmentation offload (TSO)
- [ ] Receive-side scaling (RSS)
- [ ] Fine-grained kernel locking
- [ ] Preemptible kernel (CONFIG_PREEMPT)
- [ ] Interrupt threading
- [ ] Benchmark suite (lmbench, hackbench, will-it-scale)

### Phase 13: Firmware & Platform
- [ ] UEFI runtime services (SetVariable, GetTime, etc.)
- [ ] ACPI AML bytecode interpreter
- [ ] Power management (suspend-to-RAM, hibernate)
- [ ] PCI Express (AER, hotplug, SR-IOV)
- [ ] ATA / SCSI pass-through
- [ ] NVDIMM / persistent memory
- [ ] x86 platform drivers (Samsung, Lenovo, Dell specific)
- [ ] Microcode loading
- [ ] SMBIOS table parsing
- [ ] Device tree support (for non-x86)
- [ ] ARM64 architecture port

### Phase 14: Tooling & Infrastructure
- [ ] Unit test framework (kunit-style)
- [x] Integration tests (QEMU smoke tests) — `scripts/qemu-test.sh`
- [ ] Coverage reporting (gcov)
- [ ] Kconfig / menuconfig build configuration
- [ ] Bootable ISO generation
- [ ] initramfs / initrd support
- [ ] Package manager for userland
- [ ] Cross-compiler build scripts (crosstool-NG style)
- [ ] Docker / container support (via namespaces + cgroups)
- [ ] Self-hosted build (building DIT-OS on DIT-OS)
- [x] CI/CD with QEMU boot tests on every commit
- [ ] Debuginfod / symbol server
- [ ] Documentation site (architecture, API, driver writing)
- [x] Coding style checker in CI

### Phase 15: Production Hardening
- [ ] Kernel lives in a separate address space (higher-half kernel)
- [ ] Kernel page table isolation (KPTI) for Meltdown mitigation
- [ ] Retpolines / IBRS / IBPB for Spectre mitigation
- [ ] GFNI/VAES acceleration for crypto
- [ ] Random seed early supply (RDRAND, RDSEED)
- [ ] Credential / security model for users
- [ ] ACL / extended attributes
- [ ] POSIX 1003.1e capabilities
- [ ] Audit subsystem (auditd-compatible)
- [ ] TPM 2.0 integration (measure boot, key storage)
- [ ] Full POSIX conformance certification target
