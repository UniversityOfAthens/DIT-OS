# Handoff — resume at M47

**Status: COMPLETE for M31/L7. `develop` = 8a41588 (M31/L7), one commit
ahead of `origin/develop` = aa348cc. Tree clean. Not pushed, not released.**

## Goal
Continue DIT-OS hardening on `develop`. Every step carries its SemVer bump:
patch = fix/hardening, minor = user-visible tier, major = `1.0.0`.

## Done (latest first)
- **M31/L7 (8a41588)** — syscall ABI structs pinned:
  - `fs::dirent` (264 B) and `fs::stat` (76 B) are `KERNEL_PACKED` with
    pinned sizes (`kernel/include/fs/dirent.hpp`, `fs/stat.hpp`).
  - Userland mirrors use `DIT_PACKED` + `DIT_STATIC_ASSERT`
    (`userland/include/syscall.h`, `userland/libc/include/sys/stat.h`).
  - `kernel/syscall/abi_layout.cpp` includes both sides and asserts every
    field offset; it is the only TU with userland include paths, which are
    target-scoped in `kernel/Makefile`.
  - Negative proof: reordering `dit_dirent` fields while keeping size 264
    fails the build with four `offset differs` errors.
- **M28 (aa348cc)** — `SYSCALL_BOUNCE_SIZE` 1 KiB; `DIT_SYSCALL_BOUNCE_SIZE`;
  `test_bounce_buffer`.
- **M17/execve batch (1e2eabc, pushed)** — userland `errno`, execve `.bss`
  mapping and image-window restriction, M47 tracked.
- `0.1.0-alpha` released 2026-11-03; `0.1.1-alpha` / `0.1.2-alpha` not cut.

## Next steps (in order)
1. Push `develop` when asked (`git push origin develop`).
2. **M47** (patch): `open()` ignores flags. Define O_RDONLY/O_WRONLY/O_RDWR
   (+O_CREAT/O_TRUNC), pass through `sys_open`/`vfs_open`, store on
   `fs::file`, enforce in read/write. Add a negative self-test (write to an
   O_RDONLY fd must fail) and a CVE pass.
3. **M10** (patch): reject `ata_read_sectors` counts > 255; handle
   0-means-256 explicitly.
4. Releases `0.1.1-alpha` and `0.1.2-alpha`: version strings, dated
   CHANGELOG sections, TODO rows → released, tags, PR to `main`.
5. Then: shell redirections, M19, preemption, 0.2.0 POSIX timers + shm.

## How to verify
```bash
rm -rf kernel/build bootloader/build userland/build
make -C kernel && make -C bootloader all
scripts/qemu-test.sh          # 24/24 PASS, 0 [selftest] FAIL
scripts/cppcheck.sh           # clean
pipx run --spec shellcheck-py shellcheck -S style scripts/*.sh
pipx run clang-format==20.1.8 --dry-run -Werror <changed files>
```

## Gotchas
- `_Static_assert` is rejected by the C++17 kernel build; shared headers use
  `DIT_STATIC_ASSERT`.
- Only `abi_layout.cpp` may see userland headers; keep those `-I` paths
  target-scoped in `kernel/Makefile`.
- `read()` may return less than requested (1 KiB per-call max); loop to 0.
- Kernel stack is 8 KiB; don't grow `SYSCALL_BOUNCE_SIZE` without checking
  the syscall → VFS call chain.
- execve accepts only segments inside `0x00400000`–`0x00500000`.
- qemu-test gates: anchored `^` patterns, any `[selftest] FAIL` fails.
- Cooperative scheduling: reap forked helpers; stray SIGCHLD breaks tests.
- Never commit `handoff.md` unless asked, never force-push, commit/push only
  on request.
