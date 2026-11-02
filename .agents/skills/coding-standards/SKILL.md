---
name: coding-standards
description: Formatting, naming, types, headers, comments and error-handling conventions shared by all DIT-OS C, C++ and NASM code. Load before editing any source file.
---

# Coding standards

When this file and the surrounding code disagree, match the surrounding code
and mention the inconsistency; do not reformat files you are not changing.

## Formatting

- `.clang-format` is authoritative: 4-space indent, no tabs, Allman braces,
  80 columns, `PointerAlignment: Left` (`char* p`).
- Format only the files you change:
  `pipx run clang-format==20.1.8 -i <files>` (CI pins 20.1.8).
- No trailing whitespace; every file ends with a newline (CI checks both).

## Naming

| Element | Convention | Example |
|---------|------------|---------|
| Functions, variables | `snake_case` | `vmm_map_user_page`, `page_count` |
| Types (C++) | `PascalCase` (newer code) | `Task`, `WaitQueue`, `SyscallArgs` |
| Types (VFS, C-style) | `snake_case` structs, as existing | `fs::inode`, `fs::dentry` |
| Constants | `UPPER_SNAKE_CASE`, `constexpr` | `USER_STACK_TOP` |
| Macros | `UPPER_SNAKE_CASE`, prefixed | `KERNEL_PACKED`, `KASSERT` |
| File-static globals | `s_` prefix | `s_max_file_cap` |
| Enum values | `PascalCase` for `enum class`, `UPPER` otherwise | `TaskState::Blocked`, `SYS_read` |

Do not name kernel identifiers after libc macros (`SEEK_SET`, `EOF`,
`NULL`, `errno` ...): static analysers and any libc header will expand them.

## Types

- Fixed-width types for sizes, addresses and hardware values: `uint8_t` ...
  `uint64_t`, `int32_t`, `size_t`, `uintptr_t`. Never `int`/`long` for sizes.
- Addresses as integers are `uintptr_t` (user pointers in syscalls stay
  `uintptr_t` until checked by `mm::` uaccess helpers).
- `bool` for flags; named `constexpr` bit masks for hardware bits:
  ```cpp
  constexpr uint32_t PAGE_PRESENT = 1u << 0;
  constexpr uint32_t PAGE_WRITE   = 1u << 1;
  constexpr uint32_t PAGE_USER    = 1u << 2;
  ```

## Headers

- `#ifndef` guards, never `#pragma once`. Guard = prefix + path:
  `kernel/include/mm/vmm.hpp` → `KERNEL_MM_VMM_HPP`. Prefixes: `KERNEL_`,
  `BOOT_`, `USER_`. Close with `#endif // GUARD`.
- Include order: own header, project headers (quoted), then freestanding
  system headers (`<stdint.h>`, `<stddef.h>`, `<stdarg.h>`).
- Headers declare; `.cpp` files define. `inline`/`constexpr` in headers only
  for small hot helpers.

## Comments and docs

- File header: `// DIT-OS <component> - <purpose>`.
- Public functions: a `/** ... */` comment with purpose, parameters
  (`@name`), return value and error codes (see `mm/uaccess.hpp`).
- Inline comments explain *why* (hardware quirk, spec reference, invariant),
  not *what*. Cite the Intel SDM, OSDev wiki or ELF spec where relevant.
- `// TODO:` must point at a `TODO.md` or `ISSUES.md` entry.
- No commented-out code. No dead code without an `#if 0` and a reason.

## Errors

- Kernel: return `0`/positive on success, `-errno` (`errno_defs.hpp`) on a
  recoverable error. `core::panic`/`KASSERT` only for broken kernel
  invariants — never for something a user process can trigger.
- Userland: return `-1` / negative error and let the caller decide.
- Check every return value. Error paths release everything acquired.

## Language rules

- C++17 freestanding (kernel), C11 (userland), NASM (asm).
- No exceptions, RTTI, `new`/`delete`, STL, VLAs, floating point.
- `reinterpret_cast`/`static_cast` over C-style casts in new C++ code.
- `volatile` only for MMIO and memory the compiler cannot see change; it is
  not a synchronisation primitive.
- No magic numbers: name addresses, sizes and sector counts in headers or
  `bootloader/src/common/constants.inc`.
