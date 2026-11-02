---
name: kernel-cpp
description: Writing C++17 freestanding kernel code for DIT-OS — layout, namespaces, real kernel APIs (panic, KASSERT, printk, kmalloc, uaccess, irq_save), init order, inline asm, interrupts and concurrency. Load before editing kernel/**/*.cpp or *.hpp.
---

# Kernel C++

## Layout

| Path | Contents |
|------|----------|
| `kernel/arch/x86_64/boot/` | `entry.asm`, multiboot headers, `main.cpp` (`kernel_main`), `linker.ld`, `user_image.asm` |
| `kernel/arch/x86_64/cpu/` | GDT, IDT, TSS, PIC, APIC, MSR, CPUID, SMP stubs |
| `kernel/arch/x86_64/mm/` | paging, PMM, VMM, heap, slab, `uaccess.cpp` |
| `kernel/arch/x86_64/dev/` | serial, keyboard, timer, RTC, PCI, ATA/AHCI/NVMe, floppy |
| `kernel/core/` | panic, printk, console, cmdline, kassert |
| `kernel/sched/` | tasks, scheduler, fork, exit, PIDs, mutex, `context.asm` |
| `kernel/syscall/` | dispatcher, table, fd table, TTY, one `sys_*.cpp` per syscall |
| `kernel/fs/` | `vfs.cpp`, `ramfs/` |
| `kernel/include/` | headers, mirrored by subsystem |

The directory says `x86_64`; the code is 32-bit. Never use `-m64`.

## Namespaces

Top level, one per subsystem: `core`, `mm`, `arch`, `dev`, `sched`, `fs`,
`ramfs`, `syscall`, `gfx`. Do not introduce a `kernel::` wrapper. Symbols
called from assembly are `extern "C"` with no leading underscore.

## APIs to use (check the header before calling)

| Need | Use | Header |
|------|-----|--------|
| Fatal error | `core::panic(msg)`, `core::panicf(fmt, ...)` | `core/panic.hpp` |
| Invariant | `KASSERT(cond)`, `KASSERT_MSG(cond, msg)` | `core/kassert.hpp` |
| Log | `core::printk`, `printk_info`, `printk_warn`, `printk_err` | `core/printk.hpp` |
| Raw serial | `dev::serial_puts` | `dev/serial.hpp` |
| Heap | `mm::kmalloc(size)`, `mm::kfree(ptr)` | `mm/kmalloc.hpp` |
| User memory | `mm::user_range_ok`, `copy_from_user`, `copy_to_user`, `strncpy_from_user` | `mm/uaccess.hpp` |
| Address spaces | `mm::vmm_*` | `mm/vmm.hpp` |
| IRQ masking | `arch::irq_save()`, `arch::irq_restore(flags)` | `arch/x86_64/irqflags.hpp` |
| Port I/O | helpers in `arch/x86_64/io.hpp` | |
| Sleeping lock | `sched::Mutex` | `sched/mutex.hpp` |
| Blocking | `sched::sleep_on(WaitQueue*)` and wake helpers | `sched/wait.hpp`, `sched/sched.hpp` |
| Signals | `sched::signal_send_pid`, `sched::signal_check_current` | `sched/signal.hpp` |
| Errors | `EFAULT`, `EINVAL`, ... | `errno_defs.hpp` |

Do not invent APIs. If one is missing, add it to the right header with a
doc comment and use it.

Note: `core::panic`/`panicf` are not declared `KERNEL_NORETURN` yet, so
compilers and cppcheck think code after them is reachable (`ISSUES.md`
L20).

## Initialisation order

`kernel_main` (`arch/x86_64/boot/main.cpp`) brings subsystems up in order:
serial/console → CPU (GDT, IDT, TSS, PIC) → memory (PMM, paging, VMM, heap)
→ devices (timer, keyboard) → syscalls → scheduler → ramfs → `sti` → init.
Never call into a subsystem before it is initialised; never allocate before
the heap is up (use static storage).

## Memory

- Early boot: static arrays only.
- After `heap_init`: `mm::kmalloc`/`kfree`; check every result.
- No `new`/`delete` (placement new only, on memory you allocated).
- Zero structs before use (`{}` initialisation or explicit loop); never read
  uninitialised memory.
- `alignas` for hardware structures (page tables 4 KiB, TSS, GDT).

## Interrupts and concurrency

- The kernel is uniprocessor. Shared state is protected by masking
  interrupts:
  ```cpp
  uint32_t flags = arch::irq_save();
  // short critical section, no sleeping
  arch::irq_restore(flags);
  ```
- Use `sched::Mutex` when the holder may sleep. Never sleep with IRQs
  masked.
- Restore IRQ state on every return path.
- Interrupt handlers: no allocation, no sleeping, no long loops.
- Idle/wait loops use `hlt` (or `pause`), never a bare spin.
- Signal delivery is cooperative: `sched::signal_check_current()` runs at
  syscall exit and in `sleep_on()`. A task that never makes a syscall does not
  see a pending signal, and there are no user handlers yet.

## Inline assembly

```cpp
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}
```

- Always `volatile` for side effects; declare every output, input and
  clobber; add `"memory"` when memory is read or written behind the
  compiler's back.
- Put anything longer than a few instructions in a `.asm` file.
- Comment every port and MSR with what it is.

## Forbidden

Exceptions, RTTI (`dynamic_cast`, `typeid`), STL, host libc headers,
floating point / SSE (the build uses `-mgeneral-regs-only`), VLAs,
recursion without a proven bound, and direct user-pointer dereferences.
