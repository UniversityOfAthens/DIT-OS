---
name: static-analysis
description: Run cppcheck (the required static-analysis gate) and handle its findings in DIT-OS; when and how suppressions are allowed, plus optional extra analyzers.
---

# Static analysis

## The gate: `scripts/cppcheck.sh`

Required for every C/C++ change (`AGENTS.md` §4). CI runs it in `lint.yml`.

```bash
scripts/cppcheck.sh            # gate: error, warning, portability, performance
scripts/cppcheck.sh --style    # advisory: adds style checks, never fails
```

The script:

- checks `kernel/` as C++17 and `userland/` as C11, both with
  `--platform=unix32` and `-D__GNUC__ -D__i386__`, so cppcheck sees the same
  configuration GCC builds (`KERNEL_NORETURN` and friends resolve);
- uses the project include paths (`kernel/include`, `userland/include`,
  `userland/libc/include`);
- reads suppressions from `config/cppcheck-suppressions.txt` and inline
  `// cppcheck-suppress <id>` comments;
- exits non-zero on any finding at the gated severities.

`bootloader/src/stage2/*.cpp` and `bootloader/src/uefi/` are not built and
are not checked. If you revive them, add them to the script.

The CI image is Ubuntu 24.04 (`cppcheck` 2.13 from apt). Newer local
versions may report more; fix those too or note them.

## Handling a finding

1. **Assume it is real.** Read the code path cppcheck describes. Most
   "redundant check / possible null" warnings point to a missing `noreturn`,
   a missing check, or a check in the wrong place.
2. **Fix the code** when the finding is real or when a small change makes the
   intent clear to both the tool and a reader.
3. **Suppress only verified false positives**, as narrowly as possible:
   - Prefer an inline suppression on the exact line, with the reason:
     ```cpp
     // cppcheck-suppress comparePointers ; linker symbols bound one object
     size_t n = static_cast<size_t>(image_end - image_start);
     ```
   - Use `config/cppcheck-suppressions.txt` (`id:path[:line]`) only for
     patterns that recur in a file, and put a `#` comment above each entry.
   - Never suppress by id globally, never raise the severity threshold, and
     never remove a directory from the scan to get a clean run.
4. Mention every new suppression in the security-audit report.

## Baseline (`config/cppcheck-suppressions.txt`)

The file has two sections:

- **False positives** — e.g. `comparePointers` on `end - start` of
  linker-script symbols.
- **Known baseline** — real findings that existed when the gate was added,
  each tracked in `ISSUES.md`. They are suppressed per file so the gate fails
  on anything new. **When you fix one, delete its suppression in the same
  change.** Never add to this section without an `ISSUES.md` entry.

| Id | Where | Cause | Tracked |
|----|-------|-------|---------|
| `nullPointerRedundantCheck` | `kernel/sched/sched.cpp` | `core::panic` lacks `KERNEL_NORETURN` | L20 |
| `syntaxError` | `kernel/fs/vfs.cpp` | locals named `SEEK_*` (libc macros); **the file is not analysed while suppressed** | L21 |

Lessons for new code: declare never-returning functions `KERNEL_NORETURN`;
never name identifiers after libc macros; check the index before reading
the element.

Per-file suppressions also hide new findings of the same id in that file,
so review changes to those files by hand for that bug class.

## Optional second opinions (not gates)

- `scripts/cppcheck.sh --style` — const-correctness, C-style casts, scope.
  Fix in code you touch; do not mass-fix unrelated files.
- `flawfinder userland/ kernel/` — flags risky C APIs; triage by hand.
- `clang-tidy` with `bugprone-*,cert-*,clang-analyzer-*` on a single file
  using the kernel flags.
- `gcc -fanalyzer` on userland C files.
