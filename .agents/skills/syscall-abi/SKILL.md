---
name: syscall-abi
description: The DIT-OS int 0x80 syscall ABI, the canonical syscall numbers, the kernel dispatcher pattern, user-pointer handling, and the exact steps to add a syscall. Load before editing kernel/syscall/ or userland/include/syscall.h.
---

# Syscall ABI

## Calling convention (`int 0x80`)

| Register | Role |
|----------|------|
| EAX | syscall number in; result out |
| EBX, ECX, EDX, ESI, EDI | arguments 1–5 |

- Result: `>= 0` on success, `-errno` on failure.
- The kernel preserves every register except EAX.
- The IDT gate for vector `0x80` is the only DPL-3 gate.

## Numbers

Defined in exactly two files, kept in sync by the `lint.yml` check:

- `userland/include/syscall.h` (`#define SYS_x N`)
- `kernel/include/syscall/syscall_numbers.hpp` (`enum SyscallNumber`,
  ends with `SYS_MAX`)

| # | Name | # | Name | # | Name |
|---|------|---|------|---|------|
| 0 | read | 10 | getpid | 20 | sigreturn |
| 1 | write | 11 | mmap | 21 | sigsuspend |
| 2 | open | 12 | munmap | 22 | setpgid |
| 3 | close | 13 | brk | 23 | getpgid |
| 4 | stat | 14 | kill | 24 | pipe |
| 5 | fstat | 15 | ioctl | 25 | dup |
| 6 | exit | 16 | getdents | 26 | dup2 |
| 7 | fork | 17 | sigaction | 27 | chdir |
| 8 | execve | 18 | sigprocmask | 28 | getcwd |
| 9 | waitpid | 19 | sigpending | | |

`SYS_exit` is 6 (an old header had 13). Never define a number anywhere else.
Check the headers for the current list before relying on this table.

Signals (`sched/signal.hpp`, `userland/libc/include/signal.h`):
`sigaction` installs handlers (`SA_RESTART`, `SA_NODEFER`, `SA_RESETHAND`,
`SA_NOCLDSTOP`; anything else is `-EINVAL`). Delivery pushes a
`SignalDelivery` block (restorer, signo, `SignalFrame`) below the user stack;
the userland restorer drops signo and calls `SYS_sigreturn`, which takes the
frame from the user ESP, restores it with sanitised EFLAGS (IF forced on,
IOPL 0) and bypasses the result store (it restores EAX). `sigsuspend` takes
the mask by value. `kill`/`waitpid` take signed 32-bit pids: 0 is the
caller's group, `< -1` a group, `-1` (broadcast) is `-EINVAL`. The caller may
signal itself and its descendants. Paths (`open`, `stat`, `execve`, `chdir`)
are resolved against the task's working directory by `path_from_user`.

## Kernel side

- `syscall::init()` (`kernel/syscall/syscall.cpp`) registers handlers with
  `syscall_set_handler(SYS_x, sys_x_impl)`.
- `syscall_dispatch` bounds-checks the number; unknown/unset → `-ENOSYS`.
- Handler signature:
  ```cpp
  extern SyscallResult sys_foo_impl(const SyscallArgs* args);
  ```
  `args->args[i]` are `uint64_t`; narrow them explicitly and validate.
- `sys_read`/`sys_write` copy through a `SYSCALL_BOUNCE_SIZE` (1 KiB)
  stack buffer. A read services one chunk per call (a short read is valid);
  a write loops until `count` is done or the file short-writes.
- Canonical handler shape (see `sys_read.cpp`):
  ```cpp
  uintptr_t buf = static_cast<uintptr_t>(args->args[1]);
  uint32_t count = static_cast<uint32_t>(args->args[2]);
  if (count == 0)
      return 0;
  if (!mm::user_range_ok(buf, count))
      return -EFAULT;
  char chunk[SYSCALL_BOUNCE_SIZE];          // kernel bounce buffer
  ...
  if (mm::copy_to_user(buf, chunk, n) < 0)
      return -EFAULT;
  ```
- User data always passes through a kernel buffer. VFS, ramfs and drivers
  never receive user pointers.
- Paths: `mm::strncpy_from_user(kbuf, uptr, cap)` once, then use `kbuf`.
- Structs returned to user space: zero the whole kernel copy (padding too),
  fill it, then `copy_to_user` once.

## Userland side

- Wrappers are `static inline` functions over `dit_syscall(...)` in
  `userland/include/syscall.h` (and `libc/include/sys/*.h` for grouped
  APIs). They return the raw `-errno`; on failure `errno` also gets the
  absolute value.
- Application code never writes `int $0x80` itself.

## Adding a syscall

1. Pick the next free number; add it to **both** number files (before
   `SYS_MAX` in the kernel enum).
2. Create `kernel/syscall/sys_foo.cpp` and list it in `CPP_SOURCES` in
   `kernel/Makefile` (no globbing).
3. Declare and register `sys_foo_impl` in `kernel/syscall/syscall.cpp`.
4. Add the userland wrapper.
5. Add a `test_foo()` to the init self-test, plus `test_sec_foo_*` negative
   probes (bad pointers, bad lengths, bad fds) — `security-audit` §4.
6. Update this skill's table, `CHANGELOG.md` and `TODO.md`.

## Security checklist (short form)

Number bounds-checked · every pointer through uaccess · single fetch ·
lengths checked for 0, max and wrap · signed args rejected if negative ·
fds validated · output zeroed · never returns a kernel address · cannot be
used to make the kernel panic. Full list: `security-audit` skill.
