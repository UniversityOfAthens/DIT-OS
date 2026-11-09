---
name: c-userland
description: Writing DIT-OS userland C code — how user programs are built and embedded, the user/kernel boundary, the minimal libc, the shell, and the init self-test. Load before editing userland/.
---

# C userland

## How it is built

- C11, `-ffreestanding -fno-stack-protector -fno-pie -mgeneral-regs-only
  -Wall -Wextra -Werror` (`userland/Makefile`). There is no host libc and no
  `libc.a`: needed libc sources are linked directly into each program.
- `crt0.asm` holds `_start` and must be first in the link order.
- Each program is linked with `userland/linker.ld` at the user image base
  and turned into a flat binary (`objcopy -O binary`).
- `init.bin` (init + shell) and `greeter.elf` are embedded into the kernel
  by `kernel/arch/x86_64/boot/user_image.asm`. The kernel build runs
  `make -C ../userland` itself.
- Adding a program: add it to `PROGRAMS`, its sources to `C_SOURCES` and an
  `<name>_OBJECTS` list in `userland/Makefile`; embed or seed it in the
  kernel if it must be reachable at runtime; add it to
  `USERLAND_SOURCES` in `kernel/Makefile` so the kernel rebuilds.

## Layout

```
userland/
├── crt0.asm            _start
├── linker.ld
├── include/syscall.h   numbers, dit_syscall(), inline wrappers
├── libc/include/       stdio.h, stdlib.h, sys/*.h
├── libc/src/           printf.c, malloc.c, exit.c
├── bin/init.c          PID 1: self-test, then the shell
├── bin/greeter.c       execve target used by the self-test
└── shell/              main.c, readline.c
```

## Boundary

- Never include kernel headers or call kernel functions. The only interface
  is syscalls through `userland/include/syscall.h` / `libc/include/sys/`.
- Wrappers return the raw `-errno` (there is no `errno` variable; the
  constants are in `libc/include/errno.h`). Check every return value, and
  retry a blocking call that returns `-EINTR` unless the handler used
  `SA_RESTART`.
- Signals: `libc/include/signal.h` (`sigaction` fills in the
  `__dit_sig_restorer` trampoline from `crt0.asm`).
- Inline assembly only inside the syscall wrappers.

## libc

- Small and freestanding: `printf` (to fd 1), a static 64 KiB free-list
  allocator behind `malloc`/`free` (first-fit with coalescing, mirroring the
  kernel heap; it does not use `brk` yet, and misuse ends the process), and
  `exit`. Add functions only when needed, with POSIX-compatible prototypes.
- Size arithmetic checks overflow (`n * size`), allocation failure returns
  NULL.
- `printf` bounds every conversion; never pass user/data text as the
  format.
- No hidden global state beyond what the function needs.

## Shell

- Runs in ring 3 inside `init.bin` (PID 1), reading fd 0 and writing fd 1
  through the TTY ioctl modes (`TTY_IOC_*`). It is a job-control shell:
  programs run as process groups (`fork` + `setpgid` + `execve`, `/name`
  for a bare name), `|` pipelines (up to 4 stages, 8 args each), `&`,
  `jobs`/`fg`/`bg`, `cd`/`pwd`. Foreground jobs get the console in cooked
  mode (`TTY_MODE_DEFAULT`, ISIG on) via `tcsetpgrp`; the prompt is raw.
  A `SIGCHLD` handler (SA_RESTART) records job state and reaps orphans.
- Line editing (`readline.c`) and argument splitting must stay within
  their fixed buffers; check the index **before** reading the element.
- Builtins and commands are covered by `scripts/qemu-test.sh`; add a step
  for any new one.

## Self-test (`bin/init.c`)

- `test_*()` functions run before the shell and report through
  `check(ok, what)` / `check_value(ok, value, what)`.
- `[selftest] ok` is what `qemu-test.sh` waits for; any failure prints
  `[selftest] FAIL: ...` and fails CI.
- Security probes are `test_sec_*()` (`security-audit` skill §4). Probes that
  fault run in a forked child.

## Rules

1. Static buffers unless the heap is needed; check every `malloc`.
2. Every program ends through `exit(status)`.
3. fd 0/1/2 are stdin/stdout/stderr.
4. No floating point (`-mgeneral-regs-only`).
