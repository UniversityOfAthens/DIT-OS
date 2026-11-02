# DIT-OS — Agent Guide

DIT-OS is a 32-bit x86 hobby OS with its own bootloader (no GRUB on the main
boot path), a C++17 freestanding kernel and a small C userland. This file is
the single source of truth for AI agents. Topic detail lives in skills under
`.agents/skills/` (also visible to Claude Code as `.claude/skills/`); load the
skill for the area you touch **before** editing it.

Rules here describe the code as it is. If you find a rule that no longer
matches the code, fix the rule in the same change (see "Keeping rules true").

---

## 1. Skills index

| Skill (`.agents/skills/<name>/SKILL.md`) | Load when you touch |
|------------------------------------------|---------------------|
| `handoff` | Any multi-step task (always) |
| `security-audit` | **Any code change** (mandatory, see §4) |
| `static-analysis` | Any C/C++ change (cppcheck gate) |
| `testing` | Anything that can affect build or boot |
| `coding-standards` | Any source file |
| `kernel-cpp` | `kernel/**/*.cpp`, `kernel/**/*.hpp` |
| `syscall-abi` | `kernel/syscall/`, `userland/include/syscall.h` |
| `filesystem-vfs` | `kernel/fs/`, `kernel/include/fs/` |
| `c-userland` | `userland/` |
| `nasm-assembly` | `**/*.asm`, `**/*.inc` |
| `bootloader` | `bootloader/` |
| `build-system` | Makefiles, `CMakeLists.txt`, `scripts/`, CI |
| `git-workflow` | Commits, branches, PRs |

---

## 2. Facts agents get wrong

Verify with `grep` before relying on any API; do not invent one.

- **32-bit only.** Target is i686, `-m32`. `kernel/arch/x86_64/` is a
  misnomer. Never add `-m64`, `elf64` objects or 64-bit registers.
- **Namespaces are top level**: `core`, `mm`, `sched`, `fs`, `ramfs`,
  `syscall`, `dev`, `arch`, `gfx`. There is no `kernel::` hierarchy.
- **Real APIs** (not the ones older docs invented):
  - asserts: `KASSERT(cond)`, `KASSERT_MSG(cond, msg)` (`core/kassert.hpp`)
  - logging: `core::printk`, `printk_info`, `printk_warn`, `printk_err`
  - fatal: `core::panic(msg)`, `core::panicf(fmt, ...)` — they halt forever
    but are not yet declared noreturn (`ISSUES.md` L20)
  - heap: `mm::kmalloc` / `mm::kfree` (`mm/kmalloc.hpp`)
  - user memory: `mm::user_range_ok`, `mm::copy_from_user`,
    `mm::copy_to_user`, `mm::strncpy_from_user` (`mm/uaccess.hpp`)
  - interrupts: `arch::irq_save()` / `arch::irq_restore(flags)`
    (`arch/x86_64/irqflags.hpp`). There are **no spinlocks**; the kernel is
    uniprocessor. `sched::Mutex` exists for sleeping locks.
- **User address window** is `0x00400000`–`0x01000000`
  (`mm/vmm.hpp`, `mm/uaccess.hpp`). Page 0 and the kernel are never user
  mapped. `fork()` is copy-on-write and user faults are demand paged.
- **Userland is freestanding**: flat binaries (`objcopy -O binary`) embedded
  in the kernel through `kernel/arch/x86_64/boot/user_image.asm`. `init.bin`
  also carries the shell; `greeter.elf` is seeded into ramfs for `execve`.
  There is no `libc.a`.
- **Syscalls**: `int 0x80`, number in EAX, args EBX ECX EDX ESI EDI, result in
  EAX (`>= 0` or `-errno`). Numbers live in exactly two places that CI keeps
  in sync: `userland/include/syscall.h` and
  `kernel/include/syscall/syscall_numbers.hpp`.
- **Disk layout** (sectors): 0 stage1, 2–10 stage2, 11–36 loader, 37+ stripped
  `kernel.img` (`KERNEL_SECTORS` = 200). Owned by `bootloader/Makefile`; keep
  `stage1.asm`, `stage2.asm`, `common/constants.inc` and
  `bootloader/include/boot.hpp` in sync.
- **Dead code**: `bootloader/src/uefi/` and the C++ files in
  `bootloader/src/stage2/` are not built (`ISSUES.md` H18). `boot/grub/` is
  only used by the ISO tooling.
- **Branches**: `main` (stable) and `develop` (integration). Not `dev`.
- **`tests/` is empty.** Integration testing is `scripts/qemu-test.sh` plus the
  userland self-test in `userland/bin/init.c`.

---

## 3. Commands

Run from the repository root.

```bash
# Clean build (what CI does). -Werror is always on.
rm -rf bootloader/build kernel/build userland/build
make -C kernel && make -C bootloader all

# Automated boot + shell test (exit 0 = pass). Use this, not `run`, in agents:
scripts/qemu-test.sh

# Static analysis gate (must exit 0)
scripts/cppcheck.sh

# Formatting (CI pins this version)
pipx run clang-format==20.1.8 -i <changed .cpp/.hpp/.c/.h files>

# Shell scripts
shellcheck -S style scripts/*.sh

# Interactive boot (blocks until QEMU exits; Ctrl+A then X quits)
make -C bootloader run
# GDB: QEMU waits on :1234
make -C bootloader debug
```

`make -C bootloader run` is interactive. Agents must use
`scripts/qemu-test.sh` to prove a change boots.

---

## 4. Definition of done (every change)

A change is not done, and must not be committed or reported as done, until
all of these hold. Report each result honestly, with output, in your final
message. If a step cannot run (missing tool, no QEMU), say so explicitly.

1. **Clean build passes** with `-Werror` (§3).
2. **`scripts/qemu-test.sh` passes.** New behaviour gets a new anchored check
   in that script or a new `test_*` in `userland/bin/init.c`'s self-test.
3. **`scripts/cppcheck.sh` passes** with no new findings. Never add a
   suppression without a one-line justification next to it.
   See `static-analysis`.
4. **Security audit done and written down.** Every change, addition or fix
   is audited with the `security-audit` skill: threat model, the checklist
   for each subsystem touched, and the **known-CVE regression pass** against
   `.agents/skills/security-audit/references/cve-catalog.md`. For every
   relevant CVE class the change could reintroduce, either add a negative
   regression test (a `test_sec_*` probe in the init self-test that performs
   the attack shape and expects `-EFAULT`/`-EINVAL`/process kill), or state
   why it does not apply. Findings you cannot fix go into `ISSUES.md`.
5. **Formatted** with clang-format 20.1.8; shell scripts pass shellcheck and
   use `set -euo pipefail`.
6. **Docs kept true**: `CHANGELOG.md` (`[Unreleased]`), `TODO.md`,
   `ISSUES.md`, `PROJECT_STRUCTURE.md` and these rules, where affected.

Documentation-only changes skip 1–4.

---

## 5. Handoff file (mandatory for multi-step work)

A session can end at any moment (usage limit, crash, closed terminal). Keep a
handoff current **as you go**; never leave it for the end.

- On any task with more than one step, create `handoff.md` in the repo root
  before starting and rewrite it after every meaningful step: a sub-task done,
  a decision made, a finding, or right before something long-running.
- Contents: goal; what is done; what is in progress (files touched, current
  state, half-finished edits); next steps in order; decisions and gotchas;
  how to verify. Keep it short — it is for a fresh agent with no context.
- **On start, read `handoff.md` if it exists** and resume from it instead of
  starting over. Confirm its claims against `git status` / `git diff`.
- When the task is fully done, mark it `Status: COMPLETE` at the top.
- `handoff.md` is git-ignored. Never commit it unless the user asks.

Template and details: `.agents/skills/handoff/SKILL.md`.

---

## 6. How to work

- **Read before you write.** Open the files you will change and their callers.
  Match the surrounding style.
- **Small, surgical diffs.** Touch only what the task needs. No drive-by
  refactors, reformatting of untouched code, or speculative abstractions.
  Mention unrelated problems you notice instead of fixing them silently.
- **State assumptions.** If the request is ambiguous and the choice matters,
  ask; otherwise pick the conventional option and say so.
- **Prove it.** Build, boot-test, analyse, audit (§4). Never claim success
  from reading code alone.
- **Fail loudly.** No silent error swallowing. Kernel: `-errno` for recoverable
  errors, `core::panic` for broken invariants. Userland: return `-1`/error.
- **No magic numbers.** Name addresses, sizes, sector counts and bit flags.
- **Debug systematically.** Serial log first (milestones in `testing`), form a
  hypothesis, test it, fix the root cause. Do not paper over faults.

---

## 7. Hard rules (never)

Kernel and bootloader:

1. Never dereference a user pointer. Go through `mm::copy_from_user` /
   `copy_to_user` / `strncpy_from_user`, and fetch each user value **once**
   (copy to a kernel buffer, then validate the copy).
2. Never trust a syscall argument: check fd ranges, lengths, flags, overflow
   of `addr + len`, and signedness before use.
3. Never copy kernel memory to user space without fully initialising it
   (zero structs, including padding, before filling them).
4. Never map kernel memory user-accessible or user memory executable by the
   kernel; never return kernel addresses to user space.
5. Never use exceptions, RTTI, `new`/`delete`, STL containers, or the host
   standard library. Freestanding headers only.
6. Never use floating point or SSE: the kernel saves no FPU state.
   `-march=i686 -mgeneral-regs-only` enforces this; do not remove it.
7. Never ignore an allocator failure or an error return.
8. Never busy-wait without `pause`/`hlt`; never keep interrupts disabled
   longer than the critical section.
9. Never use `#pragma once` (use `#ifndef` guards), unbounded
   `strcpy`/`memcpy`, or variable-length arrays.
10. Never weaken `-Wall -Wextra -Werror`, the SSE ban, or a CI check to make
    a change pass.

Repository:

11. Never commit build output, secrets, IDE files or `handoff.md`.
12. Never force-push or rewrite history on `main` or `develop`. Before any
    command that discards changes (`git checkout -- <path>`, `restore`,
    `reset --hard`, `clean`, `stash drop`), run `git status` and limit it to
    the exact files you changed: others may have uncommitted work in the tree.
    Never edit code the user did not ask you to change.
13. Never commit without the user asking. Never add AI co-author trailers or
    "generated with" lines to commits; commits are authored by the user.

---

## 8. Code conventions (summary)

Full rules: `coding-standards`.

- C++17 (kernel), C11 (userland), NASM. 4 spaces, Allman braces, 80 columns,
  `PointerAlignment: Left` — clang-format (`.clang-format`) is authoritative.
- `snake_case` functions/variables, `PascalCase` types, `UPPER_SNAKE_CASE`
  constants and macros, `s_` prefix for file-static globals.
- Fixed-width types (`uint32_t`, `size_t`, `uintptr_t`) for sizes, addresses
  and hardware values.
- Header guards: `KERNEL_<PATH>_HPP`, `BOOT_...`, `USER_...`.
- File header: `// DIT-OS <component> - <purpose>`.
- Comments explain *why*. `TODO:` comments must reference `TODO.md`/`ISSUES.md`.

---

## 9. Git

Full rules: `git-workflow`.

- Branch from `develop`: `feature/`, `fix/`, `refactor/`, `docs/`, `test/`;
  `hotfix/` from `main`.
- Commit subject `<component>: <imperative summary>` (≤ 72 chars, no period),
  e.g. `kernel/mm: reject wrapping ranges in user_range_ok`. Body only when
  needed, one or two lines.
- One logical change per commit; do not mix formatting with logic.

---

## 10. Keeping rules true

- The rules must describe the actual code, not plans. When a change makes a
  rule wrong (new API, renamed file, new syscall, layout change), update
  `AGENTS.md` and the affected skill in the same change.
- `.cursor/rules/*.mdc` are thin glob-scoped pointers to the skills. Put
  content in the skill, never only in a Cursor rule.
- When an agent repeats a mistake, add a one-line rule for it in §2 or §7.
