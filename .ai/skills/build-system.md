# Build System Conventions

## Scope

Applies to all `Makefile` files, `CMakeLists.txt`, and shell scripts under `scripts/` and `bootloader/scripts/`.

## Primary Build System: GNU Make

Make is the primary build system. CMake is optional for kernel-only builds.

## Makefile Structure

### Standard Makefile Layout

```makefile
# DIT-OS <component> - Makefile
#
# Builds: <what it builds>
# Usage: make [all|clean|install]

# --- Toolchain ---
CC      = gcc
CXX     = g++
ASM     = nasm
LD      = ld
OBJCOPY = objcopy

# --- Flags ---
CFLAGS  = -std=c11 -ffreestanding -Wall -Wextra
CXXFLAGS = -std=c++17 -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra
ASFLAGS = -f elf32 -g -F dwarf
LDFLAGS = -m elf_i386 -nostdlib

# --- Directories ---
SRCDIR  = src
OBJDIR  = build
INCDIR  = include

# --- Sources and Objects ---
SRCS    = $(wildcard $(SRCDIR)/**/*.cpp)
OBJS    = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))
ASM_SRCS = $(wildcard $(SRCDIR)/**/*.asm)
ASM_OBJS = $(patsubst $(SRCDIR)/%.asm,$(OBJDIR)/%.o,$(ASM_SRCS))
DEPS    = $(OBJS:.o=.d)

# --- Targets ---
.PHONY: all clean

all: $(OBJDIR)/output.elf

$(OBJDIR)/output.elf: $(OBJS) $(ASM_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -MMD -MP -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(ASFLAGS) -I$(INCDIR) $< -o $@

clean:
	rm -rf $(OBJDIR)

-include $(DEPS)
```

## Variable Naming

| Variable | Purpose | Default |
|----------|---------|---------|
| `CC` | C compiler | `gcc` |
| `CXX` | C++ compiler | `g++` |
| `ASM` | Assembler | `nasm` |
| `LD` | Linker | `ld` |
| `OBJCOPY` | Object tool | `objcopy` |
| `SRCDIR` | Source directory | `src` |
| `OBJDIR` | Output directory | `build` |
| `INCDIR` | Include directory | `include` |
| `CFLAGS` | C compiler flags | `-Wall -Wextra` |
| `CXXFLAGS` | C++ compiler flags | `-Wall -Wextra` |
| `ASFLAGS` | Assembler flags | `-f elf32` |
| `LDFLAGS` | Linker flags | `-m elf_i386` |
| `DEPS` | Dependency files | auto-generated |
| `SRCS` | Source files | `wildcard` |
| `OBJS` | Object files | `patsubst` |

## Compiler Flags Reference

### C Flags (Freestanding)

```makefile
CFLAGS = \
    -std=c11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-pie \
    -fno-pic \
    -Wall \
    -Wextra \
    -Werror \
    -m32
```

### C++ Flags (Freestanding)

```makefile
CXXFLAGS = \
    -std=c++17 \
    -ffreestanding \
    -fno-rtti \
    -fno-exceptions \
    -fno-stack-protector \
    -fno-pie \
    -fno-pic \
    -Wall \
    -Wextra \
    -Werror \
    -m32
```

### Assembler Flags

```makefile
ASFLAGS = \
    -f elf32 \
    -g \
    -F dwarf
```

### Linker Flags

```makefile
LDFLAGS = \
    -m elf_i386 \
    -nostdlib \
    -T linker.ld
```

## Dependency Tracking

Always generate `.d` files for incremental builds:

```makefile
CFLAGS += -MMD -MP
-include $(DEPS)
```

This ensures Make rebuilds files when headers change.

## Target Conventions

| Target | Purpose |
|--------|---------|
| `all` | Build everything (default) |
| `clean` | Remove all build artifacts |
| `install` | Copy built files to disk image |
| `run` | Build and run in QEMU |
| `debug` | Build and run with GDB |
| `check` | Run lint/style checks |

## Shell Scripts

### Style

```bash
#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# Variables in UPPER_CASE
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly KERNEL_DIR="${SCRIPT_DIR}/../kernel"
readonly BUILD_DIR="${KERNEL_DIR}/build"

# Functions in snake_case
build_kernel() {
    echo "Building kernel..."
    make -C "$KERNEL_DIR" all
}

# Main
main() {
    build_kernel
    echo "Done."
}

main "$@"
```

### Script Rules

1. Always use `set -euo pipefail`
2. Quote all variable expansions
3. Use `readonly` for constants
4. Functions before main logic
5. Return non-zero on error
6. Output to stderr for errors: `>&2 echo "Error"`

## Disk Image Creation

```bash
# Create floppy disk image (1.44 MiB)
dd if=/dev/zero of=disk.img bs=512 count=2880

# Write bootloader to MBR (first sector)
dd if=stage1.bin of=disk.img conv=notrunc bs=512 seek=0

# Write kernel to subsequent sectors
dd if=kernel.elf of=disk.img conv=notrunc bs=512 seek=1
```

## QEMU Invocation

```bash
qemu-system-i386 \
    -fda disk.img \
    -boot a \
    -m 128M \
    -s -S \
    -serial stdio \
    -monitor none
```

| Flag | Purpose |
|------|---------|
| `-fda` | Floppy disk image |
| `-boot a` | Boot from floppy |
| `-m 128M` | Memory size |
| `-s -S` | GDB on port 1234, wait for connection |
| `-serial stdio` | Serial output to terminal |
| `-monitor none` | Disable QEMU monitor |

## Nix Flakes

For reproducible dev environments. Use `flake.nix` at project root:

```bash
nix develop          # Enter dev shell
nix build            # Build using nix
```

Always use the nix flake when available, it ensures exact toolchain versions.

## Post-change verification

After substantive edits, run the full sequence in **`verify-build.md`** (clean `kernel/build` and `bootloader/build`, then `make -C kernel`, `make -C bootloader all`, `make -C bootloader run`). Same content as `.cursor/rules/verify-build.mdc`.

## Absolute Rules

1. **Always use `-Wall -Wextra -Werror`** - warnings are errors
2. **Always generate dependency files** (`-MMD -MP`) - prevents stale builds
3. **Never hardcode paths** - use variables and relative paths
4. **Always clean before a release build** - ensure deterministic output
5. **Never modify source files from Makefile** - only generate artifacts
6. **Always verify build succeeds before run target** - do not run broken builds
7. **Use `@mkdir -p` before writing to output** - ensure directory exists
