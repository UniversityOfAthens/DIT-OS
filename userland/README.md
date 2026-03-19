# DIT-OS Userland

Minimal userland for future ring-3 execution.

- **include/syscall.h** – Syscall ABI (int 0x80): `read`, `write`, `exit` wrappers.
- The **shell** currently runs as the kernel init task (see `kernel/sched/shell.cpp`) and uses the same syscall layer via `syscall::syscall_read/write/exit`. To run a true user-mode shell: build a standalone binary linked at a user VA, embed it in the kernel or load from initrd, map it with user pages, and iret to ring 3 (see docs/NEXT_STEPS.md).
