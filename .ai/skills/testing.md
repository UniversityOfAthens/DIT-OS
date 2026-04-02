# Testing Guidelines

## Scope

Applies to all testing activities in DIT-OS: unit tests, integration tests, and smoke tests.

## Current State

Testing is planned but not yet fully implemented. CI performs a smoke boot test (kernel + bootloader build, QEMU boot, check for panic/output).

## Testing Philosophy

1. **Test early** - test subsystems as they are built
2. **Test in isolation** - unit test individual functions where possible
3. **Test integration** - verify subsystems work together
4. **Test regressions** - fix that is not tested is a regression waiting to happen

## Test Directory Structure

```
tests/
├── unit/               # Unit tests (compiled for host)
│   ├── kernel/
│   │   ├── test_mm.cpp
│   │   ├── test_sched.cpp
│   │   └── test_vfs.cpp
│   └── userland/
│       ├── test_string.c
│       ├── test_printf.c
│       └── test_memory.c
├── integration/        # Integration tests (run in QEMU)
│   └── test_boot.py
├── scripts/            # Test runners and helpers
│   ├── run_qemu_tests.sh
│   └── check_boot.py
└── Makefile            # Test build rules
```

## Unit Tests

### Framework

Use a minimal custom test framework (no external dependencies). See `tests/framework/`.

### Test File Pattern

```cpp
// tests/unit/kernel/test_mm.cpp

#include "framework/test.hpp"
#include "mm/paging.hpp"

TEST(page_align) {
    ASSERT_EQ(kernel::mm::PAGE_ALIGN(0), 0u);
    ASSERT_EQ(kernel::mm::PAGE_ALIGN(1), 4096u);
    ASSERT_EQ(kernel::mm::PAGE_ALIGN(4096), 4096u);
    ASSERT_EQ(kernel::mm::PAGE_ALIGN(4097), 8192u);
}

TEST(page_offset) {
    ASSERT_EQ(kernel::mm::PAGE_OFFSET(0), 0u);
    ASSERT_EQ(kernel::mm::PAGE_OFFSET(4096), 0u);
    ASSERT_EQ(kernel::mm::PAGE_OFFSET(4100), 4u);
}

int main() {
    RUN_TEST(page_align);
    RUN_TEST(page_offset);
    return report_results();
}
```

### Test Naming

- File: `test_<module>.cpp` or `test_<module>.c`
- Test function: `TEST(<descriptive_name>)`
- Assertion macros: `ASSERT_EQ`, `ASSERT_NE`, `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_NULL`, `ASSERT_NONNULL`

### What to Unit Test

| Subsystem | Testable Functions |
|-----------|-------------------|
| mm | `PAGE_ALIGN()`, `PAGE_OFFSET()`, bitmap operations |
| string | `strlen()`, `strcpy()`, `strcmp()`, `memcpy()`, `memset()` |
| printf | Format parsing, number conversion |
| data structures | Linked list insert/remove, queue operations |
| bit manipulation | `set_bit()`, `clear_bit()`, `bit_is_set()` |

### What NOT to Unit Test

- Hardware register access (requires hardware context)
- Interrupt handlers (requires CPU state)
- Full scheduler (requires memory + tasks)
- Bootloader (runs in real mode, different environment)

## Integration / Smoke Tests

### QEMU Boot Test

The primary integration test: boot the kernel in QEMU and verify expected output.

```bash
#!/usr/bin/env bash
set -euo pipefail

# Build
make -C kernel clean && make -C kernel all
make -C bootloader clean && make -C bootloader all

# Create disk image
./scripts/create-disk.sh

# Boot in QEMU with timeout
timeout 10 qemu-system-i386 -fda disk.img -boot a \
    -serial stdio -nographic -monitor none 2>&1 | tee boot.log

# Check for expected output
if grep -q "DIT-OS" boot.log && ! grep -q "PANIC" boot.log; then
    echo "PASS: Boot succeeded"
    exit 0
else
    echo "FAIL: Boot failed"
    exit 1
fi
```

### Boot Test Checks

| Check | Command | Expected |
|-------|---------|----------|
| Kernel loaded | grep output | `DIT-OS` banner |
| No panics | grep output | No `PANIC` string |
| Scheduler starts | grep output | Scheduler init message |
| Serial output works | check log is non-empty | Characters received |
| Clean shutdown | QEMU exits normally | Exit code 0 |

## Test-Driven Development

When implementing new features:

1. Write failing tests first
2. Implement minimal code to pass
3. Refactor with confidence
4. Run full test suite before committing

## Linting and Static Analysis

### clang-format

Run before every commit:

```bash
find kernel bootloader -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' | \
    xargs clang-format --dry-run --Werror
```

### Compiler Warnings as Errors

All builds use `-Wall -Wextra -Werror`. Every warning must be fixed.

### Future: clang-tidy

When the project matures, add clang-tidy with:

```yaml
Checks: >
  -*, bugprone-*, modernize-*, performance-*,
  readability-*, portability-*, concurrency-*
```

## Debugging

### QEMU + GDB

```bash
# Terminal 1: Start QEMU
qemu-system-i386 -fda disk.img -boot a -s -S -serial stdio

# Terminal 2: Connect GDB
gdb kernel/build/kernel.elf
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue
```

### GDB Useful Commands

```gdb
break <symbol>            # Set breakpoint at function
break file:line           # Set breakpoint at line
info breakpoints          # List breakpoints
continue                  # Continue execution
step                      # Step into
next                      # Step over
print <expr>              # Print value
x/<N><fmt> <addr>         # Examine memory
bt                        # Backtrace
info registers            # Show registers
```

### Serial Debugging

The kernel outputs debug info via serial port (COM1, 0x3F8). QEMU redirects this to stdout with `-serial stdio`.

```cpp
// In kernel code
core::printk(KERN_DEBUG "Entering function foo with arg %d\n", arg);
```

## CI/CD Integration

GitHub Actions runs on every push and PR:

```yaml
# .github/workflows/build.yml (simplified)
steps:
  - checkout
  - install deps (gcc, nasm, qemu)
  - make -C kernel all
  - make -C bootloader all
  - create disk image
  - smoke boot in QEMU
  - check output for errors
```

## Test Coverage Goals

| Phase | Target |
|-------|--------|
| Phase 1 (boot) | Smoke boot passes |
| Phase 2 (memory) | PMM/VMM unit tests pass |
| Phase 3 (sched) | Context switch integration test |
| Phase 4 (syscalls) | Syscall unit tests pass |
| Phase 5 (fs) | VFS read/write integration test |

## Absolute Rules

1. **Never commit code that breaks existing tests**
2. **Always run `make clean && make all` before testing** - ensures clean state
3. **Always test on QEMU, never assume correctness from code review alone**
4. **Always capture serial output** - it is the primary debug channel
5. **Never suppress warnings** - fix them or document why they are suppressed
6. **Test edge cases** - zero, max value, aligned, unaligned, null pointer
