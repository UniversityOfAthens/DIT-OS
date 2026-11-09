# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Removed

- The dead bootloader cluster: the never-assembled C++ stage2 (ELF-linked
  path), the UEFI stub, and the unused FAT32/ISO9660/PNG/BMP/A20/menu/font
  modules, their headers (`bootloader/include/*.hpp`) and `macros.inc`
  (H18). The tracked bugs in those files (H11, H12, M20-M26, M43-M45, L5)
  are gone with them; the visual-bootloader plan grows from `stage2.asm`
  instead. Nothing on the boot path changed.

### Fixed

- `M22` is closed by documentation rather than code: stage2 stays entirely
  below 1 MiB (menu at `0x7E00`, loader loaded to `0x10000`, kernel staged
  at `0x20000`), and `loader.asm` enables A20 via port `0x92` before its
  first access above 1 MiB, so stage2 has no need to touch the gate.
- `pipe()` is now subject to a global cap of eight live pipes: each buffer is
  4 KiB of the 256 KiB kernel heap, and an uncapped process could tie up
  nearly all of it with pipes (and forks for more fd tables), after which
  `fork`, `open` and `pipe` failed for every process (M46). The ninth pipe
  returns `-ENOMEM`; closing both ends releases the slot. Covered by
  `test_sec_pipe_heap_cap`, which opens pipes until the cap, forks and reaps
  under that pressure, and fails if the cap ever stops biting before the
  fd-table limit does. Patch bump for `0.1.1-alpha`.
- Stage2's kernel read recovers from transient floppy failures: a failed
  sector read now resets the disk system (`int 13h AH=00h`) and retries the
  same sector up to three times instead of dropping straight to the boot
  menu, and the LBA-to-CHS math uses named constants for the 1.44 MB floppy
  geometry it depends on. The read was already one sector per BIOS call, so
  the 127-sector EDD request limit in the original write-up never applied to
  this path; the multi-sector EDD code lives in the unassembled `disk.asm`
  (dead code, H18) (L24). Patch bump for `0.1.1-alpha`.
- The canonical line buffer moved into the TTY (`tty_read_line`), like
  Linux's line discipline: a caught signal interrupts the read with
  `-EINTR` without dropping what was typed so far, and a retry continues
  the same line; a read shorter than a completed line consumes what fits
  and leaves the rest for the next read. ISIG characters (`^C`, `^\`, `^Z`)
  flush the partial line as POSIX requires. A new `greeter ttyline` mode
  and a `qemu-test.sh` step prove a signal interrupts a console read with
  `-EINTR` and the console keeps working across it (L25). Patch bump for
  `0.1.1-alpha`.
- Children of a parent that ignores `SIGCHLD` (or sets `SA_NOCLDWAIT`) are
  reaped by the kernel instead of piling up as zombies: the exiting task
  cannot free its own kernel stack, so they are freed by a deferred reaper
  that runs at syscall exit in a live task's context, and a later
  `waitpid` reports `-ECHILD` (L23). `sigaction` now accepts `SA_NOCLDWAIT`
  (Linux value 2). Covered by `test_sigchld_autoreap`, which reaps an
  ignored child, an `SA_NOCLDWAIT` child, and ten fork/exit rounds before
  forking again. Patch bump for `0.1.2-alpha`.
- Stage2 probes the boot drive's CHS translation (`int 13h AH=08h`) instead
  of assuming the 1.44 MB floppy geometry: sectors/track and heads now come
  from the BIOS, with the floppy constants as fallback for BIOSes that
  reject the probe (H20). Verified by booting with a deliberately wrong
  fallback: the probe overrides it, and every boot exercises the probed
  geometry on the loader and kernel reads. Patch bump for `0.1.2-alpha`.
- Sessions bound job control: `setsid` (syscalls 29–30 with `getsid`) makes
  a non-group-leader the leader of its own session and group, a group
  leader cannot `setsid`, and `setpgid` can neither move a process into
  another session's group nor create a group there; `tcsetpgrp` only hands
  the console to groups of the controlling session (the boot session at
  first) (L22). Covered by `test_sessions`: a child leaves the boot
  session, a grandchild proves a leader cannot `setsid`, and the parent
  proves the boundary holds while the child is still alive. Patch bump for
  `0.1.2-alpha`.
- Userland `malloc` is now a first-fit free list with coalescing over the
  same static 64 KiB arena: `free()` reclaims blocks instead of leaking
  them, adjacent free blocks merge, and a corrupted arena or a foreign /
  double free ends the process rather than continuing with a damaged heap
  (M35). The size bound is still applied before alignment (M38), so a
  wrapping request returns NULL. Covered by `test_malloc_reuse`, which
  allocates and frees 4096-byte blocks 4096 times, checks first-fit reuse
  and neighbouring-block coalescing; against the old bump allocator it
  fails after the first 16 rounds. Patch bump for `0.1.2-alpha`.
- The shell no longer treats a failed read as end of file: `getdents()`
  returning `-ENOTDIR` (for example `ls hello.txt`) now prints
  `ls: <path>: cannot read directory`, and a failed `read()` (for example
  `cat /`) prints `<program>: read error`; both go to fd 2 so a future
  stdout redirection cannot hide them (M18). A new qemu-test step drives
  both failures. Patch bump for `0.1.2-alpha`.
- `execve` maps every page a `PT_LOAD` segment's `p_memsz` covers, not just
  its file-backed pages: `.bss` can land on a page of its own and fault on
  first write. The greeter now has a four-byte `.bss` for `errno`, so its
  `crt0` BSS clear hit an unmapped page at `0x00401000` and the execve
  self-test failed before `main`. New frames are zeroed by the allocator,
  and pages mapped for the file copy are returned unchanged, so the walk is
  idempotent. Patch bump for `0.1.2-alpha`.
- `execve` confines every `PT_LOAD` segment to the image window
  (`0x00400000`-`0x00500000`) instead of the whole user address space, so a
  segment, including its `.bss` tail, can never be mapped over the stack,
  brk or mmap regions (CVE-2017-1000253 class). A malformed image is
  rejected with `-ENOEXEC` before the offending segment is mapped. Patch
  bump for `0.1.2-alpha`.
- Userland now has the POSIX `errno` variable: `dit_syscall()` still returns
  the raw negative errno (the in-tree ABI) and, on failure, also stores its
  absolute value in a per-process `errno`. Success leaves it untouched, as
  POSIX specifies (M17). `test_errno` checks `ENOENT`, `EBADF` and `EFAULT`
  from real failing calls and that a successful call preserves the
  variable. A wrapper set that returns `-1` with `errno` set remains part of
  the full-libc work. Patch bump for `0.1.2-alpha`.
- `read()`/`write()` move up to 1 KiB per syscall instead of 256 bytes: the
  kernel bounce buffer grew to 1 KiB, which leaves the 8 KiB kernel stack
  room for the VFS call chain, and `cat` now asks for a full chunk. Userland
  exposes the size as `DIT_SYSCALL_BOUNCE_SIZE` (M28). A new
  `test_bounce_buffer` fills a pipe with one chunk and fails unless a single
  `read()` returns all 1024 bytes intact; with the old 256-byte bounce it
  reports 256 and fails. Patch bump for `0.1.2-alpha`.

## [0.1.0-alpha] - 2026-11-03

**Version bump:** minor — the first usable release: POSIX signals with
handlers, job control, pipes, `dup2`, a working directory and a shell that
runs programs.

### Added

- Signal handlers: `sigaction` (`SA_RESTART`, `SA_NODEFER`,
  `SA_RESETHAND`, `SA_NOCLDSTOP`), `sigprocmask`, `sigpending`,
  `sigsuspend` and `sigreturn` (syscalls 17–21). Handlers run on a frame
  the kernel pushes below the user stack and return through a userland
  trampoline; interrupted blocking calls return `-EINTR` or restart.
  Signals now act at IRQ/exception return to ring 3 as well as at syscall
  exit, so `^C` or `kill` reaches a task spinning in user mode.
- CPU faults raise real signals (`SIGSEGV`, `SIGILL`, `SIGFPE`, `SIGBUS`,
  `SIGTRAP`) that a handler can catch; a blocked or ignored fault signal
  reverts to its default instead of looping.
- Job control: process groups (`setpgid`, `getpgid`), `kill(0)` and
  `kill(-pgid)`, stop/continue (`SIGSTOP`, `SIGTSTP`, `SIGTTIN`, `SIGTTOU`,
  `SIGCONT`), `waitpid` with process-group selectors, `WUNTRACED` and
  `WCONTINUED`, and `SIGCHLD` to the parent on exit, stop and continue.
- Console job control: a foreground process group (`TTY_IOC_GETPGRP` /
  `TTY_IOC_SETPGRP`, `tcgetpgrp`/`tcsetpgrp`), `TTY_MODE_ISIG` (`^C`,
  `^Z`, `^\` raise `SIGINT`, `SIGTSTP`, `SIGQUIT`), and `SIGTTIN`/`SIGTTOU`
  for background readers and `tcsetpgrp` callers.
- `pipe` (anonymous, 4 KiB, atomic writes up to 512 bytes, EOF and
  `SIGPIPE`/`-EPIPE`), `dup` and `dup2` (syscalls 24–26). fds 0–2 are now
  ordinary refcounted console files, so they can be closed and redirected.
- A per-process working directory: `chdir`, `getcwd` (syscalls 27–28);
  `open`, `stat`, `execve` and `chdir` resolve relative paths, `.` and
  `..` (never above `/`). ramfs seeds a `/home` directory.
- Userland `errno.h` (all kernel error numbers) and `signal.h`.
- The shell runs programs (`/name` for a bare name) as jobs: `|`
  pipelines, `&`, `jobs`, `fg`, `bg`, `cd`, `pwd`; foreground jobs get the
  console in cooked mode with `^C`/`^Z`. A `SIGCHLD` handler records job
  state and reaps orphans. `/greeter read` reads one line (for tests).
- Self-tests for all of the above, `test_sec_*` probes (forged `sigreturn`
  frames, kernel pointers to every new syscall, fd/pgid/signal ranges,
  path escape and length, inode-less `fstat` leaks), and new
  `qemu-test.sh` steps for cd/pwd, programs, pipelines, `^C`, `^Z`/`fg`,
  background jobs, `SIGTTIN` and the shell parser limits.

### Changed

- `kill` may signal the caller's descendants (was: direct children only)
  and process groups; `kill(-1)` is still `-EINVAL`. `SIGSTOP` is no longer
  `-ENOSYS`.
- init (PID 1) discards signals with a default disposition, like Linux.
- `execve` keeps ignored dispositions, the blocked mask and pending
  signals; only caught handlers revert to the default (POSIX).
- `KERNEL_SECTORS` is 240 (was 200): the kernel image is now ~212 sectors.

### Fixed

- `close()` released the now-empty slot instead of the file, so every
  closed file (and, with pipes, every pipe end) leaked and pipes never saw
  EOF.
- `fork()` and the first process no longer fall back to the kernel's fd
  table 0 when no table is free; they fail instead.
- The interrupt entry clears the direction flag: user code (or a restored
  signal frame) could enter the kernel with `DF = 1`.
- `vfs_lseek` returns `-ESPIPE` for inode-less files instead of
  dereferencing a null inode.
- M41: orphans reparented to init are reaped while the shell waits at its
  prompt: init gets `SIGCHLD` for a reparented zombie and the shell's
  handler reaps it.
- M38: `malloc()` rejects sizes that would wrap its size arithmetic.
- `png.asm` (not assembled) now sets the wanted chunk type before each
  lookup, compares the signature against the PNG constant instead of
  itself and skips the IHDR CRC, so IHDR/IDAT can be found; the
  unimplemented inflate returns an error instead of faking success (C9,
  rest tracked as M43).
- `disk.asm` `read_lba` (not assembled) uses the caller's ES as the DAP
  buffer segment instead of hardcoding 0 (C13).

### Security

- `sigreturn` never restores CS/SS, forces `IF` on and keeps `IOPL`, `TF`,
  `NT`, `RF`, `VM`, `AC` and `ID` clear, and rejects frames with a bad
  magic or an EIP/ESP outside the user window (probe:
  `test_sec_sigreturn_forged`).
- Handlers and restorers must be user addresses; every new syscall bounds
  its 64-bit arguments before narrowing and fetches user data once.

## [0.0.12-alpha] - 2026-11-03

**Version bump:** patch — string bounds, bootloader constants and a
copy-on-write frame-reference fix.

### Fixed

- String copy loops in ramfs seeding and the shell (prompt rendering,
  completion, path normalisation) check their destination bound before
  reading the source byte, so an unterminated source can no longer be read
  one byte past its buffer (M37). The matching cppcheck suppressions are
  gone.
- Bootloader constants now match the code: `STAGE2_LOAD` is `0x7E00`,
  stage1 includes `constants.inc` and reads `STAGE2_LOAD`/`STAGE2_SECTORS`
  instead of hardcoded literals (H9), and stage2 loads the full reserved
  loader region (`LOADER_SECTORS`, 26 sectors) with a build-time size
  check instead of silently loading 2 sectors (H10).
- `vmm_clone_user_directory()` now takes a frame reference for every page
  it shares, including code-image pages the source has already made
  private. The pre-mapped image PTE used to make the clone skip
  `frame_ref()`, so a child's exit released a frame the parent still
  mapped; the next COW break could allocate that frame, zero it and lose
  the parent's writes, including the self-test failure counter, which
  masked real failures (M42). Covered by `test_cow_frame_refcount`, which
  fails on the old code.

## [0.0.11-alpha] - 2026-11-02

### Fixed

- A parent that exits (including through a fatal signal) now reparents its
  children to PID 1, and the shell drains reparented zombies before each
  prompt; PID 1 exiting now panics instead of leaving the system silent
  (M36, M40). Known residual: orphans can still pile up while init is
  blocked in `read()` (`ISSUES.md` M41).
- `copy_to_user()` validates its range read-only and writes through
  `vmm_write_user()`, so a syscall can write into a page the process has
  not touched since `fork()` instead of returning `-EFAULT`; the parent's
  copy stays untouched (M39).
- `scripts/qemu-test.sh` now fails when the serial log contains any
  `[selftest] FAIL` line: the self-test's own failure counter could be
  corrupted by a COW bug (M42, fixed in 0.0.12), which let a failing
  self-test still print `ok` (found by the M39 regression test).

### Removed

- `boot/grub/` (config, `grubenv` and the DIT PF2 theme): the custom
  stage1 → stage2 → loader path is the only boot path. A visual boot menu
  with theme support is planned on our own stack (see `TODO.md`).

## [0.0.10-alpha] - 2026-11-02

**Version bump:** patch — two syscalls (`stat`/`fstat`, `kill`), POSIX
default-action signal delivery and the cppcheck/agent tooling on top of the
0.0.9 process model. `0.1.0-alpha` is reserved for the first genuinely
usable release (signal handlers, `pipe`, `dup2`, `errno.h`, job control).

### Added

- `kill()` (`SYS_kill` 14) with POSIX default-action signal delivery: only
  the caller and its direct children may be signalled; fatal signals
  (`Term`/`Core`) set a pending bit and terminate the target at its next
  syscall exit or before it blocks, ignored signals (`SIGCHLD`, `SIGURG`,
  `SIGWINCH`) are no-ops, and stop signals return `-ENOSYS` until job
  control exists. Covered by `test_kill`, `test_fault_signal`,
  `test_sec_kill_invalid` and `test_sec_kill_parent`.
- `stat()` and `fstat()` (`SYS_stat` 4, `SYS_fstat` 5):
  `fs::vfs_stat_inode()` shares the field mapping, both handlers validate
  user ranges with `mm::user_range_ok` and zero the result struct, and
  `userland/libc/include/sys/stat.h` provides `struct stat` plus
  `S_IF*`/`S_IS*`. Covered by the init self-test, including
  `test_sec_stat_*` probes for NULL, kernel and wrapping pointers, invalid
  descriptors, and uninitialised bytes in the returned struct.
- `scripts/cppcheck.sh` static-analysis gate (kernel C++17, userland C11)
  with `config/cppcheck-suppressions.txt`, run by `lint.yml`. Pre-existing
  findings were baselined and tracked as `ISSUES.md` M37 (fixed in
  0.0.12), L20 and L21.
- Agent skills in `.agents/skills/` (also linked as `.claude/skills/`),
  including a mandatory security audit with a known-CVE regression catalog
  and a handoff-file protocol.

### Changed

- Exit statuses now use the POSIX wait encoding from `sched/wait.hpp`:
  `exit()` stores `(code & 0xff) << 8`, signal deaths store the signal
  number, and user faults report `SIGSEGV`, so `WIFEXITED`/`WEXITSTATUS` and
  `WIFSIGNALED`/`WTERMSIG` work in userland.
- `AGENTS.md` rewritten to match the code (namespaces, real kernel APIs,
  freestanding userland, `develop` branch, current boot milestones) and to
  define the definition of done: clean build, `qemu-test.sh`, cppcheck and
  a written security audit.
- `.cursor/rules/` reduced to thin glob-scoped pointers to the skills.
- Pull request template asks for test output and the security-audit report.

### Removed

- `.ai/skills/`, replaced by `.agents/skills/`.

## [0.0.9-alpha] - 2026-09-26

Ring 3, demand paging and the POSIX process model: `fork`, `execve`, `waitpid`
and a heap for user space.

### Added

- User page faults are demand paged: touching a page the process owns (its
  `brk` heap, its `mmap` region, its stack) allocates a zeroed frame, and
  touching anything else still kills the process.
- `fork()`: copy-on-write address space clone plus a `fork_entry_trap` that
  restores the parent's user registers, so the child resumes just after the
  syscall with 0 and its own copy of the pages it writes.
- `execve()`: a static ELF32 loader (`syscall/sys_execve.cpp`) that reads the
  program through the VFS into a fresh address space, builds argc/argv/envp on
  a new user stack and continues in the new image instead of returning.
- `waitpid()` with `WNOHANG`, plus zombie reaping: an exiting task is kept in
  the task list until its parent collects it, and its stack, files and
  address space are released at that point.
- `brk()`, `mmap()` and `munmap()` for anonymous read/write private memory.
- Per-process file descriptor tables; a child inherits its parent's open
  files through reference counted `fs::file` objects, and a task releases its
  files on exit.
- `ramfs::seed_file()` publishes a file in the root filesystem; the boot code
  uses it to seed `/greeter` (`userland/bin/greeter.c`) for the `execve()`
  self-test.
- A boot self-test in `userland/bin/init.c` exercises demand paging, `brk`,
  `mmap`/`munmap`, `fork` (including that a child cannot change the parent's
  memory), `execve` and `waitpid`, and prints `[selftest] ok`; the QEMU
  integration test waits for that line.
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
- `copy_from_user` / `copy_to_user` / `strncpy_from_user` (`mm/uaccess`):
  `read`, `write`, `open` and `getdents` validate user pointers and return
  `-EFAULT` for NULL, low memory, wraparound or out-of-window ranges, and
  `open` returns `-ENAMETOOLONG` for over-long paths. `read`/`write` copy
  through a kernel bounce buffer so drivers and filesystems only see kernel
  memory.
- Console TTY modes: `ioctl(0, TTY_IOC_SETMODE, mode)` switches echo and
  canonical (line-edited) input on or off, so programs can read raw keys.
  `SYS_ioctl` is now implemented for fds 0-2.

- `nix develop .#cross`: dev shell with the `i686-elf` cross-compiler.
- `CROSS_COMPILE` make variable; `kernel/Makefile` uses `i686-elf-*`
  automatically when it is on `PATH` and the host GCC otherwise.

### Changed

- The kernel version string is now `DIT-OS 0.0.9-alpha` (it still said
  `0.1.0` during the previous alphas, so the banner did not match the tag).
- `mm/vmm.cpp` was rewritten around a static pool of address spaces (one page
  directory plus five page tables each) and a reference counted pool of
  physical frames for user pages, 2 MiB below 4 MiB so the kernel can reach a
  frame before any user mapping exists. The old `vmm_map`/`vmm_unmap`/
  `vmm_virt_to_phys` kernel-space helpers and `vmm_map_user`/`vmm_unmap_user`
  had no callers and are gone; `USER_SPACE_COUNT` is 8.
- Tasks are created with a kernel stack from the heap and a demand paged user
  stack in the user window, so `task_create_user()` no longer takes stacks
  from the caller.
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

- The IDT frame indices for `EIP`, `CS` and `EFLAGS` were off, so a kernel
  exception reported the segment registers and the vector instead of the
  interrupted registers. The frame layout is now spelled out in
  `arch/x86_64/idt.hpp` (`FRAME_STACK_ESP` is the user stack pointer, which is
  pushed separately from the `pusha` slot).
- The interrupted frame is now owned by the syscall layer
  (`syscall::current_regs()`) instead of a global that any interrupt taken
  during a syscall could displace.
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
