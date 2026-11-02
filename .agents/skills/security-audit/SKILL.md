---
name: security-audit
description: Mandatory security audit for every DIT-OS change, addition or fix. Threat model, per-subsystem checklists, the known-CVE regression pass, negative tests and the audit report format. Load before finishing any code change.
---

# Security audit (mandatory)

Every change, addition or fix gets this audit before it counts as done
(`AGENTS.md` §4). The audit is a written result, not a feeling: it ends with
the report in §6, included in your final message (and the PR body).

Kernel and bootloader code runs in ring 0. One unchecked length is a
privilege escalation. Assume an attacker controls every ring-3 process,
every byte of every user buffer, the disk image, and the timing of
interrupts and other processes.

## 1. Procedure

1. **Scope.** List the files and functions changed and every caller of a
   changed function (`git diff --stat`, then `grep` callers).
2. **Trust boundaries.** For each changed path, mark where untrusted data
   enters: syscall arguments, user memory, file contents and names in ramfs,
   ELF headers in `execve`, disk sectors read by the bootloader, keyboard /
   serial input, device registers.
3. **Checklists.** Walk §2 (always) and the §3 sections for every subsystem
   touched. Tick each item or explain why it does not apply.
4. **Known-CVE pass.** Open `references/cve-catalog.md`. For every bug class
   whose subsystem you touched, ask "can this change reintroduce it?" Answer
   per class. Where the answer is not a clear "no", write a regression probe
   (§4).
5. **Tools.** `scripts/cppcheck.sh` must pass (`static-analysis` skill).
   Review the diff once more for the patterns cppcheck cannot see: double
   fetch, missing permission checks, info leaks, lock/IRQ state on error
   paths.
6. **Fix or record.** Fix every finding in scope. A finding you cannot fix in
   this change goes into `ISSUES.md` with severity, location and a repro.
   Never leave a known exploitable bug unrecorded.
7. **Report** (§6).

## 2. Universal checklist

- [ ] **Bounds.** Every index and length is checked against the real
      capacity before use. Off-by-one at `cap - 1` for NUL terminators.
- [ ] **Integer overflow / wrap.** `addr + len`, `count * size`,
      `offset + size` are checked before use (`if (len > MAX - addr)`).
      Truncation from 64-bit `SyscallArgs` to 32-bit is explicit and checked.
- [ ] **Signedness.** No signed value reaches a size, index or comparison
      with an unsigned value unchecked. Negative `int` from a user is
      rejected, not cast to a huge `uint32_t`.
- [ ] **Initialisation.** No variable, struct or buffer is read before it is
      written. Structs copied to user space are zeroed first, padding
      included.
- [ ] **NULL / invalid pointers.** Every allocator result and lookup result
      is checked. No kernel path can be steered into page 0.
- [ ] **Error paths.** Every early return releases what it took (memory,
      frames, fds, refs, mutexes) and restores IRQ state.
- [ ] **Lifetime.** No use after free, double free, or dangling pointer kept
      in a global, task, fd table, dentry or wait queue after the owner dies.
      Reference counts cannot overflow.
- [ ] **Concurrency.** State shared with interrupt handlers or other tasks
      is touched with interrupts off (`arch::irq_save`/`irq_restore`) or
      under `sched::Mutex`. No check-then-use window across a sleep or
      `switch_to_next()`.
- [ ] **Resource exhaustion.** A user cannot make the kernel allocate
      unboundedly, loop forever, or panic. Failures return `-ENOMEM` /
      `-EAGAIN`, not `core::panic`.
- [ ] **Info leaks.** No kernel address, kernel stack content or another
      process's data reaches user space, the serial log of a user-triggered
      path, or an error message.
- [ ] **Format strings.** `printk`/`printf` format argument is a literal.

## 3. Subsystem checklists

### Syscall boundary (`kernel/syscall/`)
- [ ] Syscall number bounds-checked before table lookup; unset slots return
      `-ENOSYS`.
- [ ] Every user pointer goes through `mm::user_range_ok` and
      `copy_from_user`/`copy_to_user`/`strncpy_from_user`. No direct
      dereference anywhere, including in drivers/filesystems called later.
- [ ] **Single fetch.** Each user value is copied into kernel memory once,
      then validated and used from the copy (no double fetch / TOCTOU).
- [ ] Data passes through a kernel bounce buffer (`SYSCALL_BOUNCE_SIZE`);
      filesystems and drivers never see user pointers.
- [ ] fd validated (`fd_get` returns non-NULL) before use; closed fds cannot
      be used after `close`.
- [ ] Return value is `>= 0` or a negative errno; never a kernel pointer.

### Memory management (`kernel/arch/x86_64/mm/`)
- [ ] Ranges are page-aligned where required and checked for wrap and for
      overlap with the image, stack, brk and mmap regions (`mm/vmm.hpp`).
- [ ] User PTEs have `U/S` set only for user pages; kernel and frame-pool
      pages stay supervisor-only.
- [ ] Copy-on-write: a write fault on a shared frame copies before granting
      write; reference counts are updated with IRQs off; the read-only
      mapping can never be written through another path (Dirty COW class).
- [ ] Demand paging only grants pages inside regions the process owns; any
      other fault kills the process.
- [ ] TLB is flushed (`invlpg`/CR3 reload) after every permission downgrade
      or unmap.
- [ ] Freed frames are zeroed before reuse by another process.
- [ ] The user stack cannot grow into brk/mmap (stack clash class).

### Process model (`kernel/sched/`, `sys_fork`, `sys_execve`, `sys_waitpid`)
- [ ] ELF loader validates magic, class (ELF32), machine (i386), type,
      `e_phoff`/`e_phnum` within file size, and every `p_offset + p_filesz`
      and `p_vaddr + p_memsz` without overflow and inside the user window.
- [ ] `p_filesz <= p_memsz`; BSS is zeroed; segments cannot overlap the stack.
- [ ] `argv`/`envp` copy bounds both string length and count.
- [ ] A failed `execve` leaves the old image intact or kills the process
      cleanly; it never returns into a half-built address space.
- [ ] `waitpid` can only reap the caller's own children; PIDs are not reused
      while still referenced.
- [ ] Task teardown frees page directory, fds and kernel stack exactly once.

### CPU, interrupts, entry (`kernel/arch/x86_64/cpu/`, `*.asm`)
- [ ] All IDT gates are DPL 0 except the `int 0x80` gate (DPL 3).
- [ ] Entry stubs save every register, set kernel segments, and never trust
      user ESP/EFLAGS/segment values on the way back. `iret` to ring 3 uses
      a sanitised EFLAGS (IOPL 0, IF 1, no NT/VM).
- [ ] TSS `esp0` points at the current task's kernel stack on every switch.
- [ ] Exceptions from ring 3 kill the process; exceptions from ring 0 panic
      with the name, error code and `EIP`.
- [ ] No I/O port access is possible from ring 3 (IOPL 0, no I/O bitmap
      grants).

### Filesystem / VFS (`kernel/fs/`)
- [ ] Path components are length-checked; `..`, `.`, empty and repeated
      `/` resolve correctly and cannot escape the root.
- [ ] File offsets cannot go negative or overflow (`lseek` with
      `SEEK_SET`/`SEEK_CUR`/`SEEK_END`); reads past EOF return 0.
- [ ] File growth is capped (`s_max_file_cap`) and checked for overflow.
- [ ] Directory entries copied to user space are fully initialised.
- [ ] Inode/dentry freed only when nothing references it.

### Drivers and TTY (`kernel/arch/x86_64/dev/`, `kernel/syscall/tty.cpp`)
- [ ] Ring buffers check head/tail bounds; input from hardware is treated
      as untrusted (scancodes, serial bytes, ATA/AHCI responses).
- [ ] `ioctl` validates the request code and argument pointer; privileged
      requests (e.g. `TTY_IOC_POWER_OFF`) are a deliberate, documented choice.
- [ ] DMA buffers are kernel-only and physically contiguous as required.

### Bootloader (`bootloader/`)
- [ ] Every disk read checks the BIOS carry flag and retries/halts on error.
- [ ] Sector counts are bounded by the reserved area (build fails if
      `kernel.img` exceeds `KERNEL_SECTORS`).
- [ ] ELF headers parsed by the loader are bounds-checked like `execve`.
- [ ] Buffers in real mode cannot overlap the stack, stage2 or the IVT/BDA.
- [ ] Config or menu input cannot overflow fixed buffers (BootHole class).

### Userland (`userland/`)
- [ ] Line editing / argument parsing is bounded by buffer size.
- [ ] `malloc` size arithmetic checks for overflow; `brk` failure handled.
- [ ] `printf` handles every format it accepts with bounded output; user
      text is never used as a format string.
- [ ] Every syscall return value is checked.

## 4. Known-CVE regression probes

A probe is a negative test: it performs the attack shape and asserts the
kernel refuses it. Add probes to `userland/bin/init.c` as `test_sec_<name>()`
functions called from `main` before the `[selftest] ok` line. Report through
the existing `check(ok, what)` / `check_value(ok, value, what)` helpers: they
count failures, and any failure makes `scripts/qemu-test.sh` fail.

The wrappers in `userland/include/syscall.h` return the raw `-errno`.
Userland has no errno constants yet; when a probe needs one, add it to
`syscall.h` with the same value as `kernel/include/errno_defs.hpp`
(e.g. `#define EFAULT 14`) rather than hard-coding the number.

Typical probe (adapt to the change):

```c
/* Kernel address as a user buffer: must be rejected, not dereferenced. */
static void test_sec_kernel_ptr_write(void)
{
    int r = write(1, (const void*)0x00100000, 16); /* kernel image */
    check_value(r == -EFAULT, r, "write from kernel memory rejected");
}
```

- Kernel / NULL / wrapping pointers (`0`, `0x00100000`, `0xFFFFFFF0` with
  length 0x20) to every syscall that takes a buffer → `-EFAULT`.
- Lengths `0`, `1`, `cap`, `cap + 1`, `0xFFFFFFFF`, negative as `int`.
- Invalid fds: `-1`, an fd after `close`, `FD_TABLE_SIZE`
  (`kernel/include/syscall/fd.hpp`).
- Write to a read-only or COW-shared page after `fork` from the other side:
  parent must not see the child's write.
- Truncated / malformed ELF passed to `execve` → error, caller survives.
- Struct returned by `stat`/`getdents`: fill the user buffer with `0xAA`
  first and check no unexpected bytes remain in padding.

When a probe would crash or hang the whole test (e.g. a deliberate ring-3
fault), run it in a forked child and check its exit status with `waitpid`.

If a class cannot be probed from user space (e.g. bootloader parsing),
document the manual test (crafted `disk.img`, GDB session) in the report.

## 5. Static and dynamic tools

- `scripts/cppcheck.sh` — required, see `static-analysis`.
- GCC warnings are already `-Wall -Wextra -Werror`.
- `scripts/cppcheck.sh --style` (advisory) and ad-hoc `flawfinder`,
  `clang-tidy` or `gcc -fanalyzer` (C files) runs are welcome second
  opinions; they are not gates. Do not commit flag changes for them.
- QEMU + GDB (`make -C bootloader debug`) for confirming a suspected bug.

## 6. Audit report (put in final message / PR body)

```markdown
### Security audit
Scope: <files/functions changed, callers reviewed>
Trust boundaries: <where untrusted input enters>
Checklists: universal ✔, <subsystems> ✔ (exceptions: <item — reason>)
Known-CVE pass:
- <class> (<CVE>): not applicable — <reason>
- <class> (<CVE>): probe `test_sec_<name>` added, passes
Tools: cppcheck.sh clean; qemu-test.sh pass
Findings: <fixed / recorded in ISSUES.md as <id> / none>
```

Keep it short, factual and specific. "Looks safe" is not an audit result.
