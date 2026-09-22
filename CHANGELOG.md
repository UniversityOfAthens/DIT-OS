# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Shell tab completion: the first word completes builtin commands, later
  words complete file paths (a `/` is added after directories); an ambiguous
  prefix is extended to the longest common prefix, then the candidates are
  listed. Ctrl+C discards the current line and arrow keys no longer insert
  escape-sequence garbage.
- `scripts/qemu-test.sh`: QEMU integration test that drives the shell over
  serial and waits for output instead of sleeping (boot, `echo`, `cat`, `ls`,
  tab completion, a 100-command stress run, poweroff on `exit`).
- CI: new `ci.yml` builds with `-Werror`, checks the disk layout and that
  the kernel has no SSE/x87 code, runs the integration test with host GCC
  and inside `nix develop`, and builds through the CMake wrapper.
- Lint: pinned clang-format 20.1.8, shellcheck with strict mode enforced,
  BITS directives, no `#pragma once`, whitespace and final newlines,
  kernel/userland syscall numbers, and actionlint for the workflows.
- Console TTY modes: `ioctl(0, TTY_IOC_SETMODE, mode)` switches echo and
  canonical (line-edited) input on or off, so programs can read raw keys.
  `SYS_ioctl` is now implemented for fds 0-2.

- `nix develop .#cross`: dev shell with the `i686-elf` cross-compiler.
- `CROSS_COMPILE` make variable; `kernel/Makefile` uses `i686-elf-*`
  automatically when it is on `PATH` and the host GCC otherwise.

### Changed

- `build.yml` and `test.yml` are replaced by `ci.yml`. Their smoke tests
  could not fail: they looked for `OK` (already in the boot log) and
  `HELLO` (present in the echoed input).
- `release.yml` now builds and boot-tests before publishing, takes its notes
  from the matching `CHANGELOG.md` section, marks `-alpha` tags as
  pre-releases, and requests `contents: write` explicitly.
- All shell scripts use `set -euo pipefail`; `dd` errors are no longer
  hidden in `create-disk.sh`, `sign.sh` or the bootloader Makefile.
- The CPU now halts when idle: the scheduler leaves the boot stack at start
  so the real idle task (`sti; hlt`) runs, and reading from the console
  sleeps on a wait queue until the COM1 interrupt wakes it instead of
  polling the UART. QEMU went from 100% of a host core to about 1% at the
  shell prompt.
- COM1 input is interrupt driven and buffered (256 bytes), so fast or pasted
  input is no longer limited by the UART's 16-byte FIFO.
- `switch_to` saves and restores EFLAGS, and a task whose entry function
  returns now exits cleanly instead of jumping to a garbage address.
- The kernel builds with `-O2 -Werror` and generates dependency files
  (`-MMD -MP`), so header edits rebuild what includes them.
- `CMakeLists.txt` is now a thin wrapper over the Makefiles (targets `kernel`,
  `disk`, `run`, `debug`, `os-clean`) instead of a second, broken build that
  compiled 7 of the kernel's source files.

### Fixed

- The NASM lint check always passed (`grep "[BITS]"` matched any line with a
  B, I, T or S); it now checks for real `BITS` directives.
- `printk("%d")` overflowed on `INT32_MIN`.
- The COM1 interrupt line was never enabled (`MCR.OUT2` was cleared), and a
  second `serial_init()` call during boot reset the UART after setup.
- `nix develop` builds hung at CPU init: Nix's GCC emits SSE for `-m32` code,
  which faults because the kernel never enables SSE. Kernel code is now built
  with `-march=i686 -mgeneral-regs-only`, whatever the compiler's default.
- The Nix dev shell no longer applies userspace hardening flags to the kernel.
- Userland `printf("%d")` negated `INT_MIN` with undefined behaviour.

### Removed

- `s_need_resched` / `maybe_yield()`: the timer flag was only polled by the
  old busy-waiting stdin read.
- Dead code that `-Werror` now rejects: the old kernel bootstrap shell loop,
  an unused BSS-clearing function and an unused inode allocator.

## [0.0.8-alpha] - 2026-09-22

Memory, stack and crash-reporting fixes found in a code audit.

### Added

- Stack overflow detection: every task stack has a canary word that the
  scheduler checks on each switch.
- Spurious IRQ 7/15 detection in the PIC driver.

### Changed

- The disk image now carries a stripped copy of the kernel (`kernel.img`,
  about 53 KB instead of 87 KB); `kernel.bin` keeps its symbols for GDB.
- `KERNEL_SECTORS` is defined once in `bootloader/Makefile` and passed to
  stage2; the build fails if the kernel image would not fit.
- README and AGENTS.md build, run, debug and boot-milestone instructions now
  match the real build.

### Fixed

- The filesystem stopped working after about 100 shell commands: the kernel
  heap is now a first-fit free-list allocator with real `kfree`, and path
  lookups release their dentries (each dentry holds a reference on its
  parent).
- The shell task could overflow its 2 KB stack on the `ls`/`cat` path; it now
  has 16 KB.
- CPU exceptions halted silently when running without a display; they now
  print a panic report (exception name, error code, `EIP`, `CS`, `EFLAGS`,
  `CR2`) to serial and VGA. Unhandled IRQs are logged instead of halting.
- `vfs_open` ignored a failing filesystem `open` callback.
- ramfs `write` could overflow its end position and bypass the 64 KB file
  cap.
- Path components longer than 255 characters were split into two components
  instead of being rejected.
- `kernel_stack` reserved with `resb` instead of `times ... db 0` in `.bss`
  (NASM warning on every build).

## [0.0.7-alpha] - 2026-05-13

Critical bug fixes, enforced code style and reworked CI.

### Added

- `ISSUES.md`: a full audit of the codebase (critical, high, medium and low
  priority issues) that `TODO.md` now references.
- `lint.yml` workflow: `clang-format` dry run and a check that every NASM
  file declares its `BITS`.
- Cooperative rescheduling: the timer interrupt sets `s_need_resched`, and
  blocking `read()` on stdin calls `maybe_yield()` instead of spinning.
- Nix dev shell: `xxd`, `clang-tools`, `xorriso`, `mtools` and a shell hook
  that prints the build, run and debug commands.

### Changed

- Adopted a Linux-kernel-based `.clang-format` (4-space indent, no tabs,
  left pointer alignment) and reformatted all 219 C/C++ source files.
- `build.yml` checks the MBR signature and the kernel ELF header, cleans
  properly, and runs a QEMU smoke test.
- `test.yml` runs on every push and pull request, installs its own
  dependencies and verifies that the OS boots in QEMU.
- `release.yml` packages every boot stage binary with SHA256 checksums and
  drops the dead ISO target.
- `PROJECT_STRUCTURE.md` and `TODO.md` rewritten to describe the code that
  actually exists instead of the planned 1500-file layout.
- Interrupts are enabled in `kernel_main` before the scheduler starts
  instead of inside `sched_start`.

### Fixed

- Kernel truncated on boot: stage2 now loads 200 sectors instead of 110.
- Task stacks were built in the reverse of the order `context.asm` pops
  them, so new tasks started at a garbage address.
- Undefined behaviour in interrupt dispatch: the ISR stub now passes the
  saved-register pointer explicitly, and the vector/error-code stack offsets
  are corrected.
- `disk.asm` retry counter was loaded with an address instead of `3`.
- The first E820 memory-map call used an uninitialised `EBX`.
- `kernel_stack_top` was declared as a value, so the TSS got the stack's
  contents instead of its address.
- Duplicate `outb()` in `arch/io.h` and conflicting `serial_init`
  declarations.
- Duplicate `syscall_dispatch()` in `syscall_table.cpp`.
- Hardcoded `-38` return values replaced with `ENOSYS`.

### Removed

- Legacy dead files: `kernel/main.cpp`, `kernel/kernel.asm`,
  `kernel/arch/x86_64/serial.cpp`, `kernel/arch/x86_64/printk.cpp`,
  `kernel/include/kernel.h` and `kernel/include/arch/serial.h`.
- `future.md`.

## [0.0.6-alpha] - 2026-04-06

Files you can list and read from the shell.

### Added

- File descriptor table (fds 3 to 31) with `open`, `close` and `getdents`
  syscalls, plus `read`/`write` routed through the VFS for file fds.
- ramfs directory iteration (`readdir`) and seeded demo files `hello.txt`
  and `README.txt`.
- Shell builtins `ls [path]` and `cat <file>`.
- Header scaffolding for processes, threads, signals, wait queues, timers
  and CPU masks.
- `AGENTS.md` and `.ai/skills/` guidelines for AI-assisted development.

### Fixed

- `ls` and `cat`: path copying in `open` and directory entry copying in
  `getdents`.
- The QEMU `run` target no longer passes `-no-shutdown`, so the build
  workflow's smoke test can exit.

## [0.0.5-alpha] - 2026-03-19

First userland shell running on the kernel.

### Added

- Round-robin scheduler with an idle task, an init task, context switching,
  PID allocation and task exit.
- Syscall layer: dispatcher and table (`read`, `write`, `exit`, `getpid`,
  `fork` stub) reachable through `int 0x80` and direct C wrappers.
- Minimal VFS (filesystem registration, mounting, path lookup) and an
  in-memory ramfs mounted at `/`.
- VGA text-mode and framebuffer drivers.
- Userland: `init`, a shell with `echo`, `export`, a Bash-style customisable
  `PS1` prompt and `exit` (powers off QEMU), and a small libc (`printf`,
  `malloc`, `exit`).
- Header scaffolding for devices, USB, filesystems and graphics.
- Nix flake development shell.
- `AUTHORS.md` and more editor rulesets (VFS, syscall ABI, userland,
  build, testing).

### Fixed

- Boot loops: the kernel is always rebuilt, objects are built with
  `-fno-pie -fno-pic`, and QEMU runs with `-no-reboot -m 32M`.
- Crashes from the host compiler's stack protector
  (`-fno-stack-protector`).
- Libc `exit()` called a syscall wrapper that no longer existed.
- Missing build directory when linking the kernel.

## [0.0.4-alpha] - 2026-02-23

The custom bootloader boots the kernel.

### Added

- Protected-mode loader at `0x10000` that enables A20, loads a flat GDT,
  parses the kernel ELF, copies its `PT_LOAD` segments, zeroes BSS and jumps
  to the entry point with Multiboot registers set.
- Stage2 loads the loader and the kernel image from disk and hands off to
  the loader.
- Freestanding kernel headers: `errno`, `limits`, `ctype`, `string`,
  `stdlib`, `types`, kernel config defaults.
- `scripts/build.sh`, `scripts/create-disk.sh` and `scripts/run-qemu.sh`.

## [0.0.3-alpha] - 2026-02-22

Kernel skeleton.

### Added

- x86 CPU setup: GDT, IDT with 256 ISR stubs, TSS, 8259 PIC, CPUID, MSR and
  control-register helpers, plus APIC, IOAPIC and SMP stubs.
- Drivers: serial (COM1 to COM4), PS/2 keyboard and mouse, PIT timer, CMOS
  RTC, PCI enumeration and ATA PIO, plus AHCI, NVMe and floppy stubs.
- Memory management: bitmap physical allocator, paging helpers and a bump
  heap, plus VMM, slab and DMA stubs.
- Core: `printk`, console, panic, assertions, command-line parser, version
  and logo.
- Multiboot1 and Multiboot2 headers and an SMP trampoline.
- Bootloader C++ headers and stage2 C++ sources (ELF, FAT, ext2, ISO9660),
  plus E820 memory detection, VESA video and long-mode code.
- Bootloader `build.sh` and `sign.sh` scripts.
- GitHub Actions build, test and release workflows, and issue and pull
  request templates.
- Cursor editor rulesets.

### Changed

- The active stage2 build went back to the lean text menu; the graphical
  modules stay in the tree but are not assembled.
- `README.md`, `TODO.md` and `CMakeLists.txt` updated for the new layout.

## [0.0.2-alpha] - 2026-02-15

Custom bootloader.

### Added

- Stage1: 512-byte MBR that loads stage2 and logs to serial and screen.
- Stage2: text boot menu (Boot, Recovery, Reboot) that works with arrow
  keys, number keys and serial VT100 input.
- GRUB-style graphical modules: VESA graphics, bitmap and PF2 fonts, BMP and
  PNG decoders, theme engine, mouse, FAT32 reader, Multiboot support and a
  UEFI stub.
- First kernel entry point, linker script and VGA/serial output.
- `CMakeLists.txt`.

## [0.0.1-alpha] - 2026-02-14

Project setup.

### Added

- Repository with GPL-3.0 license, `README.md` and `.gitignore`.
- `PROJECT_STRUCTURE.md`, `TODO.md` and a design roadmap.
- `.clang-format` and a starter GitHub Actions workflow.
- GRUB configuration and the DIT boot theme with Ubuntu PF2 fonts.
- Bootloader configuration file.

[Unreleased]: https://github.com/UniversityOfAthens/DIT-OS/compare/72c4520...HEAD
[0.0.8-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/d55cb7d...bc5a947
[0.0.7-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/4be1fe6...d55cb7d
[0.0.6-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/79c0f18...4be1fe6
[0.0.5-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/8630a0e...79c0f18
[0.0.4-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/bef405d...8630a0e
[0.0.3-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/7e6b75d...bef405d
[0.0.2-alpha]: https://github.com/UniversityOfAthens/DIT-OS/compare/9b7397e...7e6b75d
[0.0.1-alpha]: https://github.com/UniversityOfAthens/DIT-OS/tree/9b7397e
