# Universal Coding Standards

## Core Principles

1. **Bare-metal mindset**: No standard library, no runtime, no frameworks. Everything is from scratch.
2. **Safety first**: OS code runs with full privileges. Validate all inputs, check all bounds.
3. **Clarity over cleverness**: Readable code is maintainable code. Avoid obscure tricks.
4. **Fail explicitly**: Never silently ignore errors. Panic, return error codes, or log.

## Language-Specific Standards

| Language | Standard | Compiler Flags |
|----------|----------|----------------|
| C++ (kernel, bootloader) | C++17, freestanding | `-fno-rtti -fno-exceptions -ffreestanding` |
| C (userland) | C11, freestanding | `-ffreestanding` |
| NASM | x86 32-bit | `-f elf32` |

## Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Functions | `snake_case` | `kernel_early_init()` |
| Variables | `snake_case` | `page_table_entry` |
| Types/structs | `PascalCase` | `PageTable`, `Task` |
| Macros | `UPPER_SNAKE_CASE` with prefix | `KERNEL_PACKED`, `BOOT_MAGIC` |
| Static globals | `s_` prefix | `s_run_queue`, `s_idle` |
| Namespaces | `snake_case` | `kernel::`, `boot::` |
| Enum values | `UPPER_SNAKE_CASE` with type prefix | `TASK_RUNNING`, `TASK_SLEEPING` |
| Constants | `UPPER_SNAKE_CASE` | `PAGE_SIZE`, `KERNEL_BASE` |

## Type Aliases

Use project-defined fixed-width types. Never use raw `int`, `long`, etc.

```cpp
// Kernel / Bootloader (C++)
u8, u16, u32, u64
s8, s16, s32, s64

// Userland (C)
uint8_t, uint16_t, uint32_t, uint64_t
int8_t, int16_t, int32_t, int64_t
```

## Header Guards

Always use `#ifndef`/`#define`/`#endif` guards. Never use `#pragma once`.

```cpp
#ifndef KERNEL_CORE_PANIC_HPP
#define KERNEL_CORE_PANIC_HPP

// ...

#endif // KERNEL_CORE_PANIC_HPP
```

Prefix: `KERNEL_` for kernel, `BOOT_` for bootloader, `USER_` for userland.

## Code Formatting

- Column limit: **80 characters**
- Indentation: **4 spaces**, no tabs
- Braces: Allman style (opening brace on new line)
- Spaces around operators: `a + b`, not `a+b`
- Trailing whitespace: none
- Final newline: required

Run `clang-format` before committing. Configuration is in `.clang-format` at project root.

## Include Order

1. Project headers (quoted, relative paths)
2. C standard library headers (angle brackets)
3. No external libraries (none exist in this project)

```cpp
#include "core/panic.hpp"
#include "mm/paging.hpp"

#include <stdint.h>
#include <stdarg.h>
```

## Compiler Attributes

Use centralized macros from `kernel/include/core/compiler.hpp`:

| Macro | Purpose |
|-------|---------|
| `KERNEL_PACKED` | Struct packing |
| `KERNEL_NORETURN` | Function never returns |
| `KERNEL_PRINTF(fmt, args)` | Format string checking |
| `KERNEL_ALWAYS_INLINE` | Force inlining |
| `KERNEL_UNUSED` | Suppress unused warnings |
| `KERNEL_WEAK` | Weak symbol |

## Error Handling

- **Kernel**: Use `panic()` for unrecoverable errors. Return error codes (negative errno) for recoverable ones.
- **Bootloader**: Use `panic()` or halt. Bootloader cannot recover from most errors.
- **Userland**: Return error codes. Set `errno` where applicable.

## Documentation

- File header: `// DIT-OS <component> - <description>`
- Function comments: brief description of purpose, parameters, return value
- Complex logic: inline `//` comments explaining *why*, not *what*
- TODO items: `// TODO: description` with reference to TODO.md if applicable

## Absolute Rules

1. **No dynamic allocation in bootloader** - bootloader runs in real/protected mode with limited memory
2. **No floating point in kernel** - unless explicitly implementing FPU support
3. **No C++ exceptions or RTTI** - disabled at compiler level
4. **No standard library in kernel/bootloader** - freestanding only
5. **No magic numbers** - define named constants
6. **No dead code** - remove or document with `#if 0` and explanation
7. **No platform-specific code without abstraction** - use arch/ directories for architecture-specific code
