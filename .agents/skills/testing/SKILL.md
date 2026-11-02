---
name: testing
description: Build, boot-test and debug DIT-OS — the clean-build sequence, scripts/qemu-test.sh, the init self-test, serial milestones, QEMU flags, GDB, and how to add tests. Load for any change that can affect build or boot.
---

# Testing

Never claim a change works from reading code. Build it, boot it, read the
serial log.

## 1. Verify sequence (run from repo root)

```bash
rm -rf bootloader/build kernel/build userland/build
make -C kernel             # builds userland too; -> bootloader/build/kernel.bin
make -C bootloader all     # -> bootloader/build/disk.img
scripts/qemu-test.sh       # boots and drives the shell; exit 0 = pass
scripts/cppcheck.sh        # static-analysis gate
```

- Always clean first: stale objects hide link errors.
- `make -C bootloader run` is **interactive** (blocks until QEMU exits);
  agents use `scripts/qemu-test.sh` instead.
- Doc-only changes may skip this. When in doubt, run it.

## 2. What exists

| Layer | Where | Notes |
|-------|-------|-------|
| Integration | `scripts/qemu-test.sh` | Boots `disk.img`, sends keys over serial, waits for regexes |
| User-space self-test | `userland/bin/init.c` (`test_*`) | Runs before the shell; prints `[selftest] ok` or `[selftest] N failure(s)` |
| Unit tests | `tests/` | **Empty.** No host unit-test harness exists yet |
| CI | `.github/workflows/ci.yml`, `lint.yml` | Clean `-Werror` build, disk layout, no SSE/x87, qemu-test (host GCC + Nix), cppcheck, formatting, shellcheck |

## 3. Serial milestones

Each stage prints to COM1. A missing line means the fault is in the step
**before** it.

1. `[1/3] Stage1...` — BIOS loaded stage1
2. `[2/3] Stage2 loaded.` — stage2 in real mode; boot menu waits for Enter
   (send `\r` when scripting)
3. `[3/3] Loading loader+kernel...` then `Read OK.` — disk reads done
4. `L`, `O`, `E`, `K` — loader started, protected mode, kernel ELF found,
   jumping to kernel
5. `DIT-OS Kernel starting...` — kernel C++ running
6. `[sched] init: done` — scheduler and init task created
7. `[selftest] ok` — userland self-test passed
8. Shell prompt — userland shell running

A CPU exception prints `*** KERNEL PANIC ***` with the exception name, error
code and `EIP`.

## 4. Adding tests

- **New user-visible behaviour** → new `test_<feature>()` in `init.c`,
  reporting through `check(ok, what)` / `check_value(ok, value, what)`.
  If the output must be checked from outside, print a `[selftest] ...` line
  and add a step in `qemu-test.sh`.
- **Security regression** → `test_sec_<name>()` probe
  (`security-audit` skill §4). Run probes that fault or kill the process in
  a forked child and check the status with `waitpid`.
- **Shell behaviour** → a `run_expect` step in `qemu-test.sh`.
- In `qemu-test.sh`, anchor every pattern with `^` so the echoed input line
  cannot match. Never use fixed `sleep`s; use `expect_after`/`run_expect`.
- Cover edge cases: 0, 1, max, max+1, negative, NULL, unaligned, wrapping.
- A test that cannot fail is not a test: break the code once locally and see
  the test fail before trusting it.

## 5. QEMU

What `make -C bootloader run` runs:

```bash
qemu-system-i386 -accel tcg,thread=single \
  -drive format=raw,file=bootloader/build/disk.img,index=0,if=floppy \
  -display none -serial mon:stdio -no-reboot -m 32M
```

- `-no-reboot` stops on triple fault instead of looping; add
  `-no-shutdown` to keep the VM for inspection (`scripts/run-qemu.sh` does).
- Ctrl+A then X quits.
- `qemu-test.sh` env: `QEMU_TEST_TIMEOUT` (per step, default 30 s),
  `QEMU_TEST_STRESS` (default 100), `QEMU_TEST_LOG` (keep the log).

## 6. GDB

```bash
make -C bootloader debug     # QEMU paused, GDB server on :1234
gdb -ex "target remote :1234" \
    -ex "symbol-file bootloader/build/kernel.bin" \
    -ex "break kernel_main" -ex "continue"
```

`kernel.bin` keeps symbols; `kernel.img` (on disk) is stripped.
For real-mode stages use `set architecture i8086` and break on `0x7c00`.

## 7. Common failures

| Symptom | Likely cause |
|---------|--------------|
| No serial output | stage1 not written at sector 0; check `dd` layout |
| Stops after `[2/3]` | Waiting for Enter, or A20 / protected-mode switch failed |
| Stops after `Read OK.` | Loader not at sector 11 or wrong load address |
| Triple fault right after `K` | GDT/IDT wrong, kernel linked/loaded at wrong address |
| `kernel.img` too big build error | Raise `KERNEL_SECTORS` in `bootloader/Makefile` (and keep the layout in sync) |
| Random corruption after a task switch | SSE/FPU code slipped in, or TSS `esp0` not updated |
| Syscall returns garbage | EAX clobbered, or numbers out of sync between the two headers |

## 8. Rules

1. Never commit code that fails the verify sequence.
2. Never weaken, skip or delete a test to make a change pass.
3. Always keep the serial log of a failure and quote the relevant lines.
4. Report results honestly: pass, fail (with output) or not run (with why).
