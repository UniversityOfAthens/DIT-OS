---
name: build-system
description: DIT-OS build system — toolchain selection, compiler flags that must not change, Makefile patterns, linker script, CMake wrapper, Nix shells, CI workflows and shell-script rules. Load before editing Makefiles, CMakeLists.txt, flake.nix, scripts/ or .github/workflows/.
---

# Build system

## Toolchain

- `kernel/Makefile` and `userland/Makefile` use `i686-elf-*` when it is on
  `PATH` (e.g. `nix develop .#cross`), otherwise host `gcc`/`g++`/`ld` with
  `-m32` (what CI installs). Override with `CROSS_COMPILE=`.
- NASM for all assembly. QEMU (`qemu-system-i386`) for running.
- `flake.nix`: default dev shell (host tools) and `.#cross` (cross
  compiler). CI evaluates both.
- `CMakeLists.txt` is a thin wrapper that calls the Makefiles; Make is the
  real build.

## Flags that must stay

Kernel (`COMMON_FLAGS` + C++):

```
-ffreestanding -fno-stack-protector -fno-pie -fno-pic -m32
-march=i686 -mgeneral-regs-only -O2 -Wall -Wextra -Werror -MMD -MP
-std=c++17 -fno-rtti -fno-exceptions
```

Link: `ld -m elf_i386 -T arch/x86_64/boot/linker.ld -z noexecstack`.

Userland: `-m32 -std=c11 -ffreestanding -fno-stack-protector -fno-pie
-fno-pic -mgeneral-regs-only -O2 -Wall -Wextra -Werror -MMD -MP`.

- `-march=i686 -mgeneral-regs-only` keeps SSE/x87 out (some GCCs emit SSE
  for `-m32`; the kernel saves no FPU state). CI disassembles the kernel to
  check.
- Never remove `-Werror`, never add `-w`/`-Wno-*` to silence a warning: fix
  the code. Any new flag needs a comment saying why.
- `-fno-stack-protector` is required because there is no
  `__stack_chk_fail`/canary setup. Adding stack protection means
  implementing those first.

## Linker script (`kernel/arch/x86_64/boot/linker.ld`)

Kernel at physical `0x100000`, entry `kernel_entry`, sections
`.multiboot`, `.text`, `.rodata`, `.data`, `.bss`. Linker symbols such as
`user_image_start/end` bound embedded images.

## Makefile patterns

- List sources explicitly (`CPP_SOURCES`, `ASM_SOURCES`, `C_SOURCES`);
  never glob.
- Objects under `build/`; `@mkdir -p $(dir $@)` before writing.
- `-MMD -MP` and `-include` the `.d` files so header changes rebuild.
- Object rules depend on `Makefile` so flag changes rebuild.
- `$(MAKE) -C dir`, never plain `make`, for recursion.
- `.PHONY` for every non-file target.
- Use variables for paths and tools; no absolute paths.
- A Makefile only writes build artifacts; it never edits sources.

## Shell scripts (`scripts/`, `bootloader/scripts/`)

```bash
#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly SCRIPT_DIR
```

- `set -euo pipefail` is mandatory (CI checks it); shellcheck `-S style`
  must pass.
- Quote every expansion; `readonly` constants; functions before `main`;
  `main "$@"` at the end.
- Errors to stderr, non-zero exit.
- Resolve paths from `SCRIPT_DIR`; use `mktemp -d` + `trap cleanup EXIT`
  for temp files.

## CI (`.github/workflows/`)

| Workflow | Checks |
|----------|--------|
| `ci.yml` | Clean `-Werror` build, disk layout, no SSE/x87, `qemu-test.sh` (host GCC and Nix), CMake wrapper |
| `lint.yml` | clang-format 20.1.8, shellcheck + strict mode, BITS directives, no `#pragma once`, whitespace/newlines, syscall numbers match, actionlint |
| `release.yml` | `v*` tags: build, test, release notes from `CHANGELOG.md` |

Workflow rules: pin action versions and tool versions, keep
`permissions: contents: read` unless more is needed, set `timeout-minutes`,
and never use `pull_request_target` with a checkout of PR code.
