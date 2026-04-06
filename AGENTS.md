# DIT-OS — Agent Guidelines

This file consolidates all rulesets from `.ai/skills/` and `.cursor/rules/` into a single reference for AI agents working on the DIT-OS operating system project.

---

## Project Overview

- **What**: A custom 32-bit x86 operating system with a custom bootloader (no GRUB)
- **Boot path**: Stage1 (512-byte MBR) → Stage2 (menu + kernel loader) → Loader → Kernel
- **Architecture**: x86 32-bit (`i686-elf` toolchain). The `arch/x86_64/` path is a misnomer — do NOT use `-m64`
- **Layout**: `bootloader/`, `kernel/`, `userland/`, `scripts/`, `tools/`, `config/`
- **See**: `PROJECT_STRUCTURE.md`, `TODO.md` for full directory maps

---

## Post-Change Verification

After any substantive change to kernel, bootloader, userland blobs, or build scripts, run from the **repository root**:

```bash
rm -rf bootloader/build && rm -rf kernel/build
make -C kernel
make -C bootloader all
make -C bootloader run
```

- Clean removes stale objects so link errors surface immediately
- `make -C kernel` produces `bootloader/build/kernel.bin`
- `make -C bootloader all` assembles `disk.img` and binaries
- `make -C bootloader run` launches QEMU; confirm the menu loads and kernel reaches the shell

Skip only for documentation-only changes. When in doubt, run it.

---

## Universal Coding Standards

### Core Principles
1. **Bare-metal mindset**: No standard library, no runtime, no frameworks. Everything from scratch.
2. **Safety first**: OS code runs with full privileges. Validate all inputs, check all bounds.
3. **Clarity over cleverness**: Readable code is maintainable code.
4. **Fail explicitly**: Never silently ignore errors. Panic, return error codes, or log.

### Language Matrix

| Language | Standard | Compiler Flags |
|----------|----------|----------------|
| C++ (kernel, bootloader) | C++17, freestanding | `-fno-rtti -fno-exceptions -ffreestanding -m32` |
| C (userland) | C11, links against custom libc | `-m32 -std=c11` (NOT freestanding) |
| NASM | x86 32-bit | `-f elf32` or `-f bin` (bootloader flat binaries) |

### Naming Conventions

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

### Type System

- **Kernel / Bootloader (C++)**: `u8`, `u16`, `u32`, `u64`, `s8`, `s16`, `s32`, `s64`
- **Userland (C)**: `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t`, `int8_t`, etc.
- Never use raw `int`, `long`, `char` for sizes or hardware values

### Header Guards

Always use `#ifndef`/`#define`/`#endif`. **Never use `#pragma once`**.

```cpp
#ifndef KERNEL_CORE_PANIC_HPP
#define KERNEL_CORE_PANIC_HPP
// ...
#endif // KERNEL_CORE_PANIC_HPP
```

Prefix: `KERNEL_` for kernel, `BOOT_` for bootloader, `USER_` for userland.

### Code Formatting

- Column limit: **80 characters**
- Indentation: **4 spaces**, no tabs
- Braces: **Allman style** (opening brace on new line)
- Spaces around operators: `a + b`, not `a+b`
- No trailing whitespace
- Final newline: required
- Run `clang-format` before committing

### Include Order

1. Project headers (quoted, relative paths)
2. C/C++ standard library headers (angle brackets)

```cpp
#include "core/panic.hpp"
#include "mm/paging.hpp"

#include <stdint.h>
#include <stdarg.h>
```

### Compiler Attributes

Use centralized macros from `kernel/include/core/compiler.hpp`:

| Macro | Purpose |
|-------|---------|
| `KERNEL_PACKED` | Struct packing |
| `KERNEL_NORETURN` | Function never returns |
| `KERNEL_PRINTF(fmt, args)` | Format string checking |
| `KERNEL_ALWAYS_INLINE` | Force inlining |
| `KERNEL_UNUSED` | Suppress unused warnings |
| `KERNEL_WEAK` | Weak symbol |

### Error Handling

- **Kernel**: `panic()` for unrecoverable errors; negative errno for recoverable
- **Bootloader**: `panic()` or halt — cannot recover
- **Userland**: Return error codes, set `errno`

### Documentation

- File header: `// DIT-OS <component> - <description>`
- Function comments: purpose, parameters, return value
- Complex logic: inline `//` comments explaining *why*, not *what*
- TODO items: `// TODO: description` with reference to TODO.md

---

## C++ Kernel Development

### Namespaces

All kernel code within `kernel` namespace hierarchy:

```
kernel::core     — Core kernel functionality
kernel::mm       — Memory management
kernel::sched    — Scheduler
kernel::dev      — Device drivers
kernel::fs       — Filesystem
kernel::syscall  — System calls
```

Bootloader code uses `namespace boot`.

### No Standard Library

Freestanding means **no** `<iostream>`, `<string>`, `<vector>`, `<map>`, `<exception>`, `<typeinfo>`.

**Allowed** freestanding headers in C++17:
- `<cstdint>`, `<cstddef>`, `<climits>`, `<cfloat>`
- `<type_traits>` (subset), `<utility>` (limited), `<initializer_list>`
- `<new>` (placement new only), `<cstdarg>`

When in doubt, implement functionality in `kernel/lib/`.

### Memory Management

- **No heap allocation early in boot** — use static arrays, stack allocation, placement new
- **When heap available**: use `mm::kmalloc()` / `mm::kfree()`, never raw `new`/`delete`
- Use `alignas()` for structures requiring specific alignment

### Pointers and Casting

- Use `reinterpret_cast` for hardware addresses, not C-style casts
- Use `volatile` for memory-mapped I/O
- Always check pointers from memory allocator before use

### Inline Assembly

```cpp
static inline void outb(u16 port, u8 value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}
```

Rules:
- Always use `volatile` for I/O
- List all clobbered registers
- Use `"memory"` clobber when asm affects memory
- Prefer named input/output operands

### constexpr and Templates

- Use `constexpr` liberally for constants and compile-time computations
- Templates are allowed in freestanding mode
- Avoid exception-throwing template code and overly complex metaprogramming

### Bit Manipulation

Use explicit bit operations with named constants:

```cpp
constexpr u32 PAGE_PRESENT = (1 << 0);
constexpr u32 PAGE_WRITE   = (1 << 1);
constexpr u32 PAGE_USER    = (1 << 2);
```

### Logging

- Use `core::printk()` with log levels (`KERN_EMERG` through `KERN_DEBUG`)
- Use `KERNEL_ASSERT` for internal consistency checks

### Kernel Initialization Order

1. Early init: Serial console, basic memory detection
2. CPU init: GDT, IDT, TSS, PIC/APIC
3. Memory init: PMM, VMM, heap
4. Device init: Timer, keyboard, disk controllers
5. Filesystem init: VFS, ramfs
6. Scheduler init: Task system, first user process

Never access a subsystem before it is initialized.

### Kernel Absolute Rules

1. No `new`/`delete` — use kernel allocators
2. No exceptions — use error codes and `panic()`
3. No RTTI — no `dynamic_cast`, `typeid`
4. No STL containers — implement or use kernel equivalents
5. No floating point — unless implementing FPU support
6. No thread-unsafe globals without protection
7. No busy-waiting without `cpu_relax()`
8. No uninitialized variables

---

## C Userland Development

### Project Structure

```
userland/
├── libc/           # C library implementation
│   ├── include/    # Public libc headers
│   ├── src/        # libc implementation
│   └── crt/
│       └── crt0.asm
├── shell/          # Shell binary
├── bin/            # Core user binaries (init, ls, cat, etc.)
└── include/
    └── syscall.h   # Syscall numbers + raw asm wrappers
```

### Hard Boundary: Userland vs Kernel

- Userland code **NEVER** includes kernel headers
- Userland code **NEVER** calls kernel functions directly
- The only kernel interface is **syscalls** via `userland/include/syscall.h`

### Error Return Convention

Syscalls and libc functions follow Unix convention:

```c
// On success: return >= 0
// On error: return -1, set errno
ssize_t sys_read(int fd, void* buf, size_t count);
```

### C Runtime (crt0)

`crt0.asm` is the real entry point (`_start`):
1. Set up stack pointer
2. Clear BSS section
3. Call `main(argc, argv)`
4. Call `exit(return_value)`

### User Binaries

- `init.c` is PID 1: mounts filesystems and `exec`s the shell
- Every binary ends with `exit(0)` or `exit(status)`
- Binaries are ELF32 executables linked against `libc.a`
- File descriptors: 0=stdin, 1=stdout, 2=stderr

### Shell

- The kernel-space `kernel/sched/shell.cpp` is a **bootstrap stub only**
- The real shell lives in `userland/shell/` and runs as a proper user process
- Communicates via fd 0 (stdin) and fd 1 (stdout), not direct serial calls
- Builtins: `cd`, `echo`, `exit`, `help` at minimum

### Userland Absolute Rules

1. No dynamic memory before heap is initialized — use static buffers
2. Never trust kernel return values blindly — always check
3. No inline assembly in userland except syscall wrappers
4. All libc functions must match POSIX prototypes
5. No globals in shared libc — reentrancy matters

---

## Syscall ABI

### Calling Convention (int 0x80, 32-bit)

| Register | Role |
|----------|------|
| EAX | Syscall number (in); return value (out) |
| EBX | Argument 1 |
| ECX | Argument 2 |
| EDX | Argument 3 |
| ESI | Argument 4 |
| EDI | Argument 5 |

- Return: EAX >= 0 on success; EAX = `-errno` on error
- Kernel saves/restores all registers except EAX
- Entry point: IDT handler for vector `0x80`

### Syscall Numbers

Defined in `userland/include/syscall.h` and `kernel/include/syscall/syscall_numbers.hpp`. **Do NOT redefine elsewhere**.

```c
#define SYS_read    0
#define SYS_write   1
#define SYS_open    2
#define SYS_close   3
#define SYS_stat    4
#define SYS_fstat   5
#define SYS_exit    6
#define SYS_fork    7
#define SYS_execve  8
#define SYS_waitpid 9
#define SYS_getpid  10
#define SYS_mmap    11
#define SYS_munmap  12
#define SYS_brk     13
#define SYS_kill    14
#define SYS_ioctl   15
```

**WARNING**: `SYS_exit` is `6`, not `13`. If you see `SYS_exit 13`, fix it.

### Adding a New Syscall

1. Add `#define SYS_foo N` to **both** `userland/include/syscall.h` and `kernel/include/syscall/syscall_numbers.hpp`
2. Create `kernel/syscall/sys_foo.cpp` with the implementation
3. Add function pointer to table in `kernel/syscall/syscall_table.cpp`
4. Add wrapper to `userland/libc/` if needed
5. Update `SYSCALL_COUNT` if compile-time constant

### Syscall Security

- Bound-check syscall number against table size
- Validate all user-supplied pointers with `copy_from_user` / `copy_to_user`
- Never dereference user-space pointers directly
- Validate fd ranges, buffer sizes, counts

---

## NASM Assembly

### Assembler Configuration

```makefile
nasm -f elf32 -g -F dwarf input.asm -o output.o
```

| Flag | Purpose |
|------|---------|
| `-f elf32` | 32-bit ELF object format |
| `-g -F dwarf` | Debug symbols for GDB |
| `-f bin` | Flat binary (bootloader stage1/stage2) |

### File Structure

```nasm
; DIT-OS <component> - <description>

; --- Constants ---
KERNEL_BASE equ 0x100000

section .data
align 4
some_data:
    dd 0x12345678

section .bss
align 16
kernel_stack:
    resb 8192

section .text
global function_name
function_name:
    ; ...
    ret
```

### Section Conventions

| Section | Usage |
|---------|-------|
| `.text` | Executable code |
| `.data` | Initialized data (read-write) |
| `.rodata` | Read-only constants |
| `.bss` | Uninitialized data (zeroed at load) |
| `.mbr` | Bootloader MBR code (stage1) |

### Naming

- **Global functions**: `snake_case`, exported with `global`
- **Local labels**: `.local_label` (prefixed with dot)
- **Constants**: `UPPER_SNAKE_CASE` with module prefix
- **Data labels**: `snake_case`

### Calling Convention

- cdecl: Caller pushes args right to left, caller cleans stack
- Frame setup: `push ebp; mov ebp, esp; sub esp, locals_size`
- Access: `[ebp+8]` = arg1, `[ebp+12]` = arg2, etc.

### Bootloader Assembly

**Stage 1 (MBR)**:
```nasm
bits 16
org 0x7C00
; 512 bytes, loaded by BIOS
times 510 - ($ - $$) db 0
dw 0xAA55    ; MBR signature
```

**Stage 2**:
```nasm
bits 16
; Loaded at 0x0500 or 0x7E00
; Responsibilities: Enable A20, set up GDT, switch to protected mode,
; jump to 32-bit code, load kernel ELF
```

### Context Switching

```nasm
context_switch:
    pushf
    pushad
    mov [edi], esp    ; edi = &old_task->kstack
    mov esp, [esi]    ; esi = &new_task->kstack
    popad
    popf
    ret
```

### Assembly Absolute Rules

1. Always specify `bits 16` or `bits 32`
2. Never use `times` in code sections — only for padding
3. Document every hardware port access
4. Never clobber callee-saved registers without saving (EBX, ESI, EDI, EBP, ESP)
5. Always include MBR signature (0xAA55) in stage1
6. Always set up GDT before protected mode switch
7. Always load kernel ELF at correct address before jump
8. Never assume register state on interrupt entry — save everything

---

## Filesystem / VFS

### Layer Architecture

```
syscall (sys_open, sys_read, ...)
        ↓
    VFS core  (kernel/fs/vfs.cpp)
        ↓
  filesystem driver  (ramfs, ext2, fat32, ...)
        ↓
   block layer / buffer cache
```

Every filesystem operation goes through VFS — **never** call a filesystem driver directly from a syscall handler.

### Core VFS Structs

All filesystem drivers must implement these operation tables:

```cpp
struct SuperblockOps {
    Inode* (*alloc_inode)(Superblock*);
    void   (*destroy_inode)(Inode*);
    int    (*sync)(Superblock*);
};

struct InodeOps {
    int    (*lookup)(Inode* dir, Dentry* target);
    int    (*create)(Inode* dir, Dentry*, int mode);
    int    (*mkdir)(Inode* dir, Dentry*, int mode);
    int    (*unlink)(Inode* dir, Dentry*);
};

struct FileOps {
    int    (*open)(Inode*, File*);
    ssize_t (*read)(File*, void* buf, size_t, off_t*);
    ssize_t (*write)(File*, const void* buf, size_t, off_t*);
    int    (*readdir)(File*, void* dirent_buf, FillDirFn);
    int    (*release)(Inode*, File*);
};
```

### Implementing a New Filesystem

1. Create `kernel/fs/<fsname>/`
2. Implement `<fsname>.cpp` with `fs_register()` and `mount()`
3. `mount()` allocates `Superblock`, fills `sb->ops`, reads on-disk super, returns root `Inode`
4. Implement `InodeOps` and `FileOps`
5. Register: `vfs_register_fs(&my_fs_type)`

### ramfs (Priority Filesystem)

- No disk backend; inodes allocated from `kmalloc`
- File data stored in memory pages
- Must support: `create`, `mkdir`, `unlink`, `read`, `write`, `readdir`

### Path Resolution

- Done by VFS `namei.cpp`, not individual filesystems
- Filesystems only implement `lookup(dir_inode, name)` → `Inode*`
- Never implement full path walking inside a filesystem driver

### Buffer Cache

- Block-based filesystems must go through buffer cache (`kernel/fs/buffer.cpp`)
- `buffer_read(dev, block_nr)` → returns `Buffer*`
- Call `buffer_release(buf)` when done

### Error Codes

| Code | Meaning |
|------|---------|
| `-ENOENT` | File not found |
| `-EEXIST` | File already exists |
| `-EISDIR` / `-ENOTDIR` | Wrong type |
| `-ENOSPC` | No space |
| `-EIO` | I/O error |

---

## Build System

### Toolchain

| Component | Compiler | Notes |
|-----------|----------|-------|
| Bootloader (stage1/stage2) | `nasm`, `i686-elf-gcc` | NASM for stage1/stage2 flat binaries |
| Kernel | `i686-elf-gcc`, `i686-elf-g++` | 32-bit, freestanding |
| Userland | `i686-elf-gcc` | Links against custom libc, NOT freestanding |

Always verify toolchain with `scripts/check-toolchain.sh` before building.

### Compiler Flags

**Kernel (C++)**:
```
-ffreestanding -fno-rtti -fno-exceptions -fno-stack-protector
-nostdlib -nostdinc -Wall -Wextra -Werror -O2 -m32 -std=c++17
```

**Bootloader stage2 (C++)**:
```
-ffreestanding -fno-rtti -fno-exceptions -fno-stack-protector
-nostdlib -m32 -std=c++17 -I bootloader/include
```

**Userland (C)**:
```
-m32 -std=c11 -Wall -Wextra -Werror -O2 -I userland/libc/include
```

### Linker Scripts

**Kernel** (`kernel/arch/x86_64/boot/linker.ld`):
- Loads at physical `0x100000` (1MB)
- Sections: `.multiboot`, `.text`, `.rodata`, `.data`, `.bss`
- Entry: `kernel_entry` (defined in `entry.asm`)

**Bootloader**: No linker script — flat binary via `nasm -f bin`

### NASM Output Formats

- Stage1 / Stage2 / Loader: `nasm -f bin` → flat binary
- Assembly linking with C++: `nasm -f elf32`
- Never mix `elf64` with `-m32` objects

### Disk Image Layout

| Offset (sectors) | Content | Size | Load Address |
|------------------|---------|------|--------------|
| 0 | stage1.bin | 512 B | 0x7C00 |
| 1 | stage2.bin | 9 sectors | 0x7E00 |
| 10 | loader.bin | variable | 0x10000 |
| after loader | kernel.bin (ELF) | variable | 0x100000 |

Sector counts and offsets **must stay in sync** between `bootloader/Makefile`, `common/constants.inc`, and `bootloader/include/boot.hpp`.

### Makefile Patterns

- Each subdirectory has its own `Makefile`
- **Never glob-compile** (`*.cpp`) — list sources explicitly
- Object files go in `build/` subdirectory
- Use `$(MAKE)` not `make` for recursive calls
- Add `.PHONY: all clean` to every Makefile
- Always generate `.d` dependency files (`-MMD -MP`)

### Target Conventions

| Target | Purpose |
|--------|---------|
| `all` | Build everything (default) |
| `clean` | Remove all build artifacts |
| `install` | Copy built files to disk image |
| `run` | Build and run in QEMU |
| `debug` | Build and run with GDB |
| `check` | Run lint/style checks |

### Shell Scripts

```bash
#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Functions in snake_case
build_kernel() { ... }

main() { ... }
main "$@"
```

Rules:
1. Always use `set -euo pipefail`
2. Quote all variable expansions
3. Use `readonly` for constants
4. Functions before main logic
5. Return non-zero on error
6. Output errors to stderr: `>&2 echo "Error"`

### Build Absolute Rules

1. **Always use `-Wall -Wextra -Werror`** — warnings are errors
2. **Always generate dependency files** (`-MMD -MP`) — prevents stale builds
3. **Never hardcode paths** — use variables and relative paths
4. **Always clean before a release build** — ensure deterministic output
5. **Never modify source files from Makefile** — only generate artifacts
6. **Always verify build succeeds before run target**
7. **Use `@mkdir -p` before writing to output** — ensure directory exists

---

## Testing

### Running the OS

```bash
# Build everything
./scripts/build.sh

# Run in QEMU (serial output to terminal)
./scripts/run-qemu.sh

# Run with GDB attached
./scripts/run-qemu-debug.sh &
./scripts/gdb.sh
```

### Serial Output Milestones

Each stage should print to COM1 (serial). If you see these, that stage is alive:

1. `[stage1] booting...` — stage1 loaded by BIOS
2. `[stage2] loaded` — stage2 running in real mode
3. `[loader] jumping to kernel` — loader about to enter kernel
4. `kernel_main: started` — kernel C code running
5. `[init] shell started` — scheduler running, init task reached

If a stage's message is missing, the fault is in the transition *before* that stage.

### QEMU Configuration

```bash
qemu-system-i386 \
  -drive file=bootloader/build/disk.img,format=raw,if=floppy \
  -serial stdio \
  -no-reboot \
  -no-shutdown \
  -m 32M
```

- `-serial stdio` captures serial output
- `-no-reboot -no-shutdown` keeps QEMU open on triple fault

### Debug Mode

```bash
qemu-system-i386 \
  -drive file=bootloader/build/disk.img,format=raw,if=floppy \
  -serial stdio -no-reboot -no-shutdown -m 32M \
  -s -S          # -s = GDB on :1234, -S = pause at start
```

### GDB

```bash
i686-elf-gdb \
  -ex "target remote :1234" \
  -ex "symbol-file bootloader/build/kernel.bin" \
  -ex "break kernel_main" \
  -ex "continue"
```

### Unit Tests

- Run on the **host** (not in QEMU) — test pure logic
- Compile with host `g++`, not cross-compiler
- No kernel headers allowed in unit tests
- Run: `make -C tests/unit && tests/unit/run_all.sh`

### Integration Tests

- Launch QEMU and check serial output
- Must complete within 10 seconds (use `timeout`)
- Exit 0 = pass, non-zero = fail

### Common Failure Modes

| Symptom | Likely Cause |
|---------|-------------|
| No serial output at all | stage1 not loaded; check disk image `dd` layout |
| Hangs after `[stage2]` | A20 or protected mode transition failed |
| Triple fault immediately | GDT/IDT wrong, or kernel loaded at wrong address |
| `[init] shell started` then hang | Scheduler not yielding; check `sti; hlt` in idle task |
| Syscall returns wrong value | EAX clobbered before return, or wrong syscall number |

### Linting

- Run `clang-format` before every commit
- All builds use `-Wall -Wextra -Werror`

### Testing Absolute Rules

1. Never commit code that breaks existing tests
2. Always run `make clean && make all` before testing
3. Always test on QEMU, never assume correctness from code review alone
4. Always capture serial output — it is the primary debug channel
5. Never suppress warnings — fix them or document why
6. Test edge cases — zero, max value, aligned, unaligned, null pointer

---

## Security

### Principle of Least Privilege

1. User code never runs in kernel mode — syscalls are the only gateway
2. Validate all user-supplied data — never trust userspace
3. Minimize interrupt-disable time — re-enable ASAP
4. Isolate address spaces — each process gets its own page tables

### Input Validation at Syscall Boundary

All data crossing the syscall boundary is untrusted:

- Validate fd ranges
- Validate buffer is in user address space with `is_user_address()`
- Validate count (check for zero, overflow)
- Use `copy_from_user` / `copy_to_user` — never dereference user pointers directly

### Memory Safety

- **Buffer overflow**: Always pass size to string/memory copy functions
- **Integer overflow**: Check arithmetic before it overflows: `if (page_count > SIZE_MAX / PAGE_SIZE)`
- **Null pointer**: Always check allocations before use

### Concurrency Safety

**Lock ordering** (highest to lowest):
1. Interrupt lock
2. Scheduler lock
3. Memory manager lock
4. Device locks
5. Filesystem locks

Never acquire locks out of order.

**Interrupt-safe pattern**:
```cpp
u32 flags;
spin_lock_irqsave(&lock, flags);
// ... critical section ...
spin_unlock_irqrestore(&lock, flags);
```

### Address Space Isolation

- Kernel pages: supervisor-only (ring 0), read/write
- User pages: user-accessible (ring 3), read/write/exec as needed
- Never map kernel pages as user-accessible
- Never return kernel addresses to userspace

### GDT Ring Levels

- Ring 0: Kernel code
- Ring 3: User code
- Never allow user code to run in ring 0

### IDT Gate Types

- Kernel interrupt gate: DPL=0, only kernel can trigger
- User syscall gate: DPL=3, user can trigger via `int 0x80`

### Security Absolute Rules

1. Never dereference user-space pointers directly — use `copy_from_user` / `copy_to_user`
2. Never disable interrupts longer than necessary
3. Never map kernel pages as user-accessible
4. Never trust syscall arguments — validate everything
5. Never use unbounded `memcpy`/`strcpy` — always pass size
6. Never return kernel addresses to userspace
7. Never allow unsigned values to wrap into negative — check bounds first
8. Never ignore error returns from memory allocators
9. Never hardcode passwords or keys in source code
10. Never log sensitive data

---

## Git Workflow

### Branch Strategy

```
main          ← stable, always builds and boots
  ├── dev     ← integration branch for features
  │   ├── feature/my-feature
  │   ├── fix/my-bugfix
  │   └── refactor/my-refactor
  └── hotfix/urgent-fix
```

### Branch Naming

| Prefix | Usage | Example |
|--------|-------|---------|
| `feature/` | New functionality | `feature/smp-scheduler` |
| `fix/` | Bug fix | `fix/page-fault-boot` |
| `refactor/` | Code restructuring | `refactor/syscall-handler` |
| `docs/` | Documentation only | `docs/memory-map` |
| `test/` | Adding tests | `test/kernel-panic` |
| `hotfix/` | Urgent fix on main | `hotfix/mbr-signature` |

### Commit Messages

Format:
```
<component>: <short summary>

<optional body>

<optional footer>
```

Rules:
1. Subject line: max 72 characters
2. Component prefix: use subsystem name
3. Imperative mood: "add feature" not "added feature"
4. No period at end of subject line
5. Blank line between subject and body

### Valid Component Prefixes

```
bootloader:     Bootloader (stage1, stage2, loader)
kernel:         Core kernel
kernel/mm:      Memory management
kernel/sched:   Scheduler
kernel/dev:     Device drivers
kernel/fs:      Filesystem
kernel/syscall: Syscall layer
userland:       Userland code
libc:           C library
shell:          Shell binary
build:          Build system / Makefiles
ci:             CI/CD workflows
nix:            Nix flake
docs:           Documentation
scripts:        Helper scripts
```

### Commit Granularity

- One logical change per commit
- Do not mix refactoring with feature additions
- Do not mix formatting changes with logic changes
- If a commit touches more than 10 files, consider splitting

### Before Committing

1. Run `make clean && make all` — full clean build
2. Run `make -C bootloader run` — verify it boots
3. Run `clang-format` on changed files
4. Review `git diff` for unintended changes

### Pre-Commit Checklist

- [ ] Builds cleanly with `-Werror`
- [ ] Boots in QEMU (if applicable)
- [ ] No debug prints left in code
- [ ] No commented-out code blocks
- [ ] Header guards are correct
- [ ] No trailing whitespace
- [ ] File ends with newline

### Pull Requests

- Title follows commit message convention: `<component>: <short summary>`
- No force push to `main` or `dev`
- At least one review before merge
- Squash merge for feature branches
- Merge commit for `dev` → `main`

### Undoing Changes

- `git revert <commit>` — safe, creates new commit
- `git commit --amend` — only on local/unpushed commits
- **Never** `git rebase -i` or `git push --force` on `main` or `dev`

### Git Absolute Rules

1. Never commit broken code — must build and boot
2. Never commit secrets — no passwords, keys, tokens
3. Never commit binary files — build generates them
4. Never commit IDE-specific files — use .gitignore
5. Always run `make clean && make all` before committing

---

## Bootloader Conventions

### Layout

- Stage1: `bootloader/src/stage1/` (stage1.asm, boot.hpp, Makefile)
- Stage2: `bootloader/src/stage2/` (asm + C++)
- Shared headers: `bootloader/include/` (boot.hpp, disk.hpp, fs.hpp, memory.hpp, video.hpp, elf.hpp, types.hpp)
- Scripts: `bootloader/scripts/` (build.sh, sign.sh)

### Includes

From `src/stage2/*.cpp` use `#include "../../include/foo.hpp"` (path relative to bootloader root).

### Build

- Primary build is NASM: `stage2.asm` → `stage2.bin`; stage1 + stage2 concatenated → bootloader.bin
- C++ stage2 files exist for future linked build; do not break current `make all`

### Disk/Kernel Loading

- Kernel is loaded at `0x100000`
- Stage2 reads from disk (CHS/LBA) per constants in `common/constants.inc` and `include/boot.hpp`
- Keep sector counts and load addresses in sync with Makefile

---

## Scripts and Tools

- **Shell**: Prefer bash; use `set -e` for build scripts. Shebang: `#!/usr/bin/env bash`
- **Paths**: Resolve project root from script location, never hardcode absolute paths
- **Invoking build**: Call the main or bootloader Makefile as appropriate
- **New scripts**: Add entries to TODO.md if part of PROJECT_STRUCTURE

---

## General Practices

- **TODO.md**: When adding or completing files that appear in PROJECT_STRUCTURE.md, update TODO.md (check off completed, add missing)
- **No magic numbers**: Use named constants or `include/` / `constants.inc` for addresses (0x7C00, 0x100000, 0xB8000), sector counts, and layout
- **Safety**: Do not disable sandboxing in scripts without good reason. Do not commit secrets or API keys
- **Documentation**: Keep file headers brief. For non-obvious algorithms or ABI details, add a short comment or reference (Multiboot spec, OSDev wiki)

---

## Universal Absolute Rules

1. No dynamic allocation in bootloader — runs in real/protected mode with limited memory
2. No floating point in kernel — unless explicitly implementing FPU support
3. No C++ exceptions or RTTI — disabled at compiler level
4. No standard library in kernel/bootloader — freestanding only
5. No magic numbers — define named constants
6. No dead code — remove or document with `#if 0` and explanation
7. No platform-specific code without abstraction — use `arch/` directories
