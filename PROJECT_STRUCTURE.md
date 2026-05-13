# DIT-OS — Project Structure

This document reflects the **actual** current state of the codebase.
Files listed as stubs or dead code exist on disk but are not compiled into the
current boot path.

---

## Directory Tree

```
DIT-OS/
├── .ai/
│   └── skills/            # AI agent skill definitions (build, coding, testing, etc.)
├── .clang-format           # Clang-format config (LLVM style — does NOT match C++ guidelines)
├── .cursor/
│   └── rules/              # Cursor IDE rules for each subsystem
├── .github/
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.md
│   │   └── feature_request.md
│   ├── PULL_REQUEST_TEMPLATE.md
│   └── workflows/
│       ├── build.yml       # CI build workflow
│       ├── hello.yaml      # Example/test workflow
│       ├── release.yml     # Release workflow
│       └── test.yml        # Test workflow
├── .gitignore
├── AGENTS.md               # AI agent guidelines (authoritative)
├── AUTHORS.md
├── CHANGELOG.md
├── CMakeLists.txt           # CMake support (experimental, uses host compiler not i686-elf)
├── ISSUES.md                # Generated issue tracker
├── LICENSE
├── PROJECT_STRUCTURE.md     # This file
├── README.md
├── TODO.md                  # Task tracking (50+ unchecked items)
├── flake.lock
├── flake.nix                # Nix flake (native gcc13 only, no i686-elf cross-compiler)
│
├── boot/                    # GRUB boot files (alternative boot path)
│   └── grub/
│       ├── grub.cfg
│       ├── grubenv
│       └── themes/dit/
│           ├── theme.txt
│           └── fonts/       # Ubuntu font PF2 files
│
├── bootloader/
│   ├── Makefile             # Top-level: builds disk.img, runs QEMU
│   ├── build/               # Build output (disk.img, stage*.bin, kernel.bin)
│   ├── config/
│   │   └── boot.cfg         # Bootloader configuration
│   ├── include/             # Shared C++ headers
│   │   ├── boot.hpp         #   Load addresses, sector counts
│   │   ├── config.hpp       #   Boot config parser
│   │   ├── disk.hpp         #   Disk I/O declarations
│   │   ├── elf.hpp          #   ELF loader declarations
│   │   ├── fs.hpp           #   Filesystem declarations
│   │   ├── memory.hpp       #   Memory map declarations
│   │   ├── types.hpp        #   Type aliases (boot:: namespace)
│   │   └── video.hpp        #   Video mode declarations
│   ├── scripts/
│   │   ├── build.sh         #   Build bootloader
│   │   └── sign.sh          #   Sign MBR with 0xAA55
│   ├── src/
│   │   ├── common/          # Shared assembly include files
│   │   │   ├── constants.inc  # CAUTION: STAGE2_LOAD=0x1000 wrong (should be 0x7E00)
│   │   │   └── macros.inc
│   │   ├── stage1/          # MBR stage1 (flat binary, 512 bytes)
│   │   │   ├── stage1.asm   #   Loads stage2 from disk via INT 13h
│   │   │   ├── boot.hpp     #   Stage1 constants
│   │   │   └── Makefile
│   │   ├── stage2/          # Stage2 — real mode menu + kernel loader
│   │   │   ├── stage2.asm   #   PRIMARY: flat binary entry point (active build path)
│   │   │   ├── a20.asm      #   A20 gate (fast method only)
│   │   │   ├── disk.asm     #   Disk read routines (INT 13h CHS + LBA)
│   │   │   ├── fat32.asm    #   FAT32 reader — BRANCH INVERTED BUG at line 198
│   │   │   ├── font.asm     #   Bitmap font rendering
│   │   │   ├── gdt.asm      #   GDT for protected mode
│   │   │   ├── graphics.asm #   VESA/VBE graphics mode
│   │   │   ├── memory.asm   #   E820 memory map — UNINITIALIZED EBX BUG
│   │   │   ├── menu.asm     #   Boot menu UI
│   │   │   ├── mouse.asm    #   PS/2 mouse (unused in flat build)
│   │   │   ├── multiboot.asm#   Multiboot info (unused)
│   │   │   ├── pf2.asm      #   PF2 font loader
│   │   │   ├── pmode.asm    #   Protected mode switch
│   │   │   ├── png.asm      #   PNG decoder — STUB (zlib does nothing)
│   │   │   ├── bmp.asm      #   BMP loader
│   │   │   ├── theme.asm    #   Theme loader
│   │   │   ├── video.asm    #   VGA/VESA helpers
│   │   │   ├── stage2_core.asm   # Core routines (ELF-linked build)
│   │   │   ├── stage2_main.asm   # Alternate entry (ELF-linked build)
│   │   │   ├── long_mode.asm     # x86-64 long mode (unused, 32-bit only)
│   │   │   ├── stage2.cpp        # C++ stage2 entry (UNUSED, not in flat build)
│   │   │   ├── console.cpp       # C++ console (UNUSED)
│   │   │   ├── disk.cpp          # C++ disk (UNUSED)
│   │   │   ├── elf.cpp           # C++ ELF loader (UNUSED)
│   │   │   ├── ext2.cpp          # C++ ext2 reader (UNUSED)
│   │   │   ├── fat.cpp           # C++ FAT reader (UNUSED)
│   │   │   ├── fs.cpp            # C++ FS abstraction (UNUSED)
│   │   │   ├── gdt.cpp           # C++ GDT (UNUSED, broken include path)
│   │   │   ├── iso9660.cpp       # C++ ISO9660 reader (UNUSED)
│   │   │   ├── memory.cpp        # C++ memory map (UNUSED)
│   │   │   ├── paging.cpp        # C++ paging (UNUSED, broken include path)
│   │   │   ├── video.cpp         # C++ video (UNUSED)
│   │   │   ├── stage2.ld         # Linker script for ELF-linked build (UNUSED)
│   │   │   └── Makefile          # Builds stage2.asm flat binary
│   │   ├── loader/          # Loader — protected mode, loads kernel ELF
│   │   │   ├── loader.asm   #   ELF parsing, kernel jump (ACTIVE)
│   │   │   ├── loader.bin   #   Pre-built binary in source
│   │   │   └── Makefile
│   │   └── uefi/
│   │       └── uefi.asm     # UEFI PE bootloader (STUB, section count wrong)
│   └── themes/dit/
│       ├── theme.txt
│       └── fonts/           # PF2 font files
│
├── config/                  # EMPTY — no Kconfig system yet
│
├── docs/                    # EMPTY — no documentation yet
│
├── initrd/                  # EMPTY — no initrd yet
│
├── iso/                     # EMPTY — no ISO generation yet
│
├── kernel/
│   ├── Makefile             # Builds kernel ELF, compiles userland sources
│   ├── main.cpp             # LEGACY — dead code, not compiled
│   ├── kernel.asm           # LEGACY — dead code, not compiled
│   ├── build/               # Object files
│   │
│   ├── include/             # Kernel headers (~80 files)
│   │   ├── types.hpp        #   kernel::u8, kernel::u16, etc.
│   │   ├── types.h          #   C-compat u8, u16 in global NS (DUAL SYSTEM)
│   │   ├── kernel.hpp       #   Umbrella -> core/kernel.hpp
│   │   ├── kernel.h         #   C-style declarations (conflicts with arch/serial.h)
│   │   ├── stdint.hpp       #   Freestanding C++ wrappers
│   │   ├── stddef.hpp
│   │   ├── stdbool.hpp
│   │   ├── stdlib.hpp
│   │   ├── string.hpp
│   │   ├── limits.hpp
│   │   ├── ctype.hpp
│   │   ├── math.hpp
│   │   ├── assert.hpp
│   │   ├── errno.hpp
│   │   ├── errno_defs.hpp       # Errno constants (missing ENOSYS=38)
│   │   ├── config.hpp
│   │   ├── kconfig.hpp
│   │   ├── kconfig_defaults.hpp
│   │   ├── arch/
│   │   │   ├── io.h             # DUPLICATE outb() — compile error
│   │   │   ├── serial.h         # Conflicting serial_init declaration
│   │   │   └── x86_64/          # Architecture headers (GDT, IDT, PIC, APIC, etc.)
│   │   ├── core/                # Panic, printk, console, config, cmdline, etc.
│   │   ├── dev/                 # Device driver interfaces
│   │   ├── fs/                  # VFS interfaces (vfs, inode, dentry, file, stat, etc.)
│   │   ├── graphics/            # VGA, framebuffer, font, color, terminal
│   │   ├── lib/                 # EMPTY — no lib headers yet
│   │   ├── mm/                  # PMM, VMM, kmalloc, slab, page, memblock
│   │   ├── sched/               # Task, sched, process, thread, signal, timer
│   │   └── syscall/             # Syscall dispatch, numbers, table, trace
│   │
│   ├── core/                    # Core kernel subsystems
│   │   ├── kernel.cpp           #   kernel_main() entry point
│   │   ├── panic.cpp            #   Kernel panic
│   │   ├── printk.cpp           #   printf to VGA + serial
│   │   ├── kassert.cpp          #   Runtime assertions
│   │   ├── console.cpp          #   Console abstraction (duplicates VGA code from printk)
│   │   ├── version.cpp          #   Version string
│   │   ├── config.cpp           #   CONFIG INIT IS NO-OP
│   │   ├── cmdline.cpp          #   Command line parsing
│   │   ├── logo.cpp             #   Boot logo
│   │   ├── modules.cpp          #   Module stub
│   │   ├── symbols.cpp          #   Symbol table stub
│   │   └── Makefile
│   │
│   ├── arch/
│   │   └── x86_64/              # ALL CODE IS 32-BIT i386 (naming is wrong)
│   │       ├── boot/            # Kernel entry point (entry.asm, multiboot headers)
│   │       │   ├── entry.asm    #   First code: BSS clear, call init
│   │       │   ├── main.cpp     #   kernel_early_init, GDT/IDT/serial setup
│   │       │   ├── init.cpp     #   kernel_init() — subsystem initialization
│   │       │   ├── linker.ld    #   Linker script (load at 0x100000)
│   │       │   ├── multiboot.asm    # Multiboot1 header
│   │       │   ├── multiboot2.asm   # Multiboot2 header
│   │       │   ├── trampoline.asm   # SMP trampoline
│   │       │   └── Makefile
│   │       ├── cpu/             # CPU initialization
│   │       │   ├── gdt.cpp/asm  #   GDT setup
│   │       │   ├── idt.cpp/asm  #   IDT + interrupt dispatch (UB at line 52)
│   │       │   ├── tss.cpp/asm  #   Task State Segment
│   │       │   ├── pic.cpp/asm  #   PIC (8259A)
│   │       │   ├── apic.cpp     #   APIC bootstrap (disabled)
│   │       │   ├── lapic.cpp    #   Local APIC (disabled)
│   │       │   ├── ioapic.cpp   #   I/O APIC (disabled)
│   │       │   ├── smp.cpp/asm  #   SMP bootstrap (disabled)
│   │       │   ├── msr.cpp/asm  #   Model Specific Registers
│   │       │   ├── cpuid.cpp/asm#   CPU feature detection
│   │       │   ├── control.cpp/asm  # Control registers
│   │       │   ├── mtrr.cpp     #   MTRR support
│   │       │   ├── pat.cpp      #   Page Attribute Table
│   │       │   ├── arch.cpp     #   arch_init() dispatcher
│   │       │   └── cpufeatures.cpp
│   │       ├── mm/              # Memory management
│   │       │   ├── pmm.cpp      #   Physical Memory Manager (bitmap)
│   │       │   ├── vmm.cpp      #   Virtual Memory Manager (stub)
│   │       │   ├── paging.cpp/asm   # Paging helpers
│   │       │   ├── heap.cpp     #   Bump allocator (kfree is no-op)
│   │       │   ├── early_mm.cpp #   Early memory init
│   │       │   ├── memblock.cpp #   Memory block tracking
│   │       │   ├── slab.cpp     #   Slab allocator stub
│   │       │   ├── dma.cpp      #   DMA allocator stub
│   │       │   ├── tlb.asm      #   TLB flushing
│   │       │   └── mtrr.asm
│   │       ├── dev/             # Device drivers
│   │       │   ├── serial.cpp   #   Serial port (COM1-4)
│   │       │   ├── keyboard.cpp #   PS/2 keyboard
│   │       │   ├── mouse.cpp    #   PS/2 mouse
│   │       │   ├── timer.cpp    #   PIT timer
│   │       │   ├── rtc.cpp      #   CMOS RTC
│   │       │   ├── pci.cpp      #   PCI bus enumeration
│   │       │   ├── ata.cpp      #   ATA PIO
│   │       │   ├── ahci.cpp     #   AHCI STUB (uses port I/O, wrong)
│   │       │   ├── nvme.cpp     #   NVMe STUB
│   │       │   └── floppy.cpp   #   Floppy STUB
│   │       ├── graphics/
│   │       │   ├── vga.cpp      #   VGA text mode
│   │       │   └── framebuffer.cpp  # Basic framebuffer
│   │       ├── serial.cpp       # LEGACY — not compiled
│   │       └── printk.cpp       # LEGACY — not compiled
│   │
│   ├── fs/                      # Filesystem
│   │   ├── vfs.cpp              #   VFS: path resolution, open/read/write/readdir
│   │   └── ramfs/               #   RAM filesystem (in-memory)
│   │       ├── ramfs.cpp/hpp
│   │       └── inode.cpp
│   │
│   ├── sched/                   # Scheduler
│   │   ├── sched.cpp            #   Round-robin scheduler
│   │   ├── task.cpp             #   Task creation (STACK INIT ORDER WRONG)
│   │   ├── context.asm          #   Context switch
│   │   ├── pid.cpp              #   PID allocation
│   │   ├── idle.cpp             #   Idle task
│   │   ├── init_task.cpp        #   Init task creator
│   │   ├── shell.cpp            #   Kernel serial shell (bootstrap)
│   │   ├── fork.cpp             #   FORK STUB (returns -ENOSYS)
│   │   ├── exit.cpp             #   Task exit
│   │   └── mutex.cpp            #   Mutex (struct not exported — dead code)
│   │
│   ├── syscall/                 # System calls
│   │   ├── syscall.cpp          #   Dispatcher
│   │   ├── syscall_table.cpp    #   Dispatch table
│   │   ├── syscall_trace.cpp    #   Trace stubs
│   │   ├── syscall_wrappers.cpp #   Kernel-internal wrappers
│   │   ├── userland_syscall_cwrap.cpp  # User-to-syscall glue (NO pointer validation)
│   │   ├── fd.cpp               #   File descriptor table
│   │   ├── sys_read.cpp         #   read()
│   │   ├── sys_write.cpp        #   write()
│   │   ├── sys_open.cpp         #   open()
│   │   ├── sys_close.cpp        #   close()
│   │   ├── sys_exit.cpp         #   exit()
│   │   ├── sys_getpid.cpp       #   getpid()
│   │   ├── sys_getdents.cpp     #   getdents()
│   │   └── sys_fork.cpp         #   fork() stub returns -ENOSYS
│   │
│   ├── mm/                      # EMPTY — MM is in arch/x86_64/mm/
│   ├── lib/                     # EMPTY
│   ├── src/                     # EMPTY
│   └── userland/                # Stale .o files (copied from ../userland)
│
├── scripts/
│   ├── build.sh                 # Build kernel + bootloader + disk image
│   ├── create-disk.sh           # Create floppy disk image (dd)
│   └── run-qemu.sh              # Run QEMU (serial stdio, 32MB)
│
├── tests/                       # EMPTY — no tests yet
│
├── tools/                       # All subdirectories are EMPTY
│   ├── build-iso/
│   ├── cross-compiler/
│   ├── font-converter/
│   └── mkinitrd/
│
└── userland/
    ├── README.md
    ├── include/
    │   └── syscall.h            # Syscall numbers + inline wrappers
    ├── crt0.asm                 # C runtime (no BSS clear — unused)
    ├── bin/
    │   ├── init.c               # Init process (mounts ramfs, starts shell)
    │   └── build/               # Stale .o artifacts (cat, ls)
    ├── shell/
    │   └── main.c               # Shell: builtins (cd, echo, exit, help, ls, cat)
    └── libc/
        ├── include/
        │   ├── stdio.h          # printf declaration
        │   └── stdlib.h         # exit, malloc declarations
        ├── src/
        │   ├── stdio/printf.c   # printf + utoa
        │   ├── stdlib/exit.c    # _exit() -> inline exit()
        │   ├── stdlib/malloc.c  # Simple bump allocator
        │   ├── string/          # EMPTY source dir (stale .o in build/)
        │   ├── unistd/          # EMPTY source dir (stale .o in build/)
        │   └── crt/             # EMPTY source dir
        └── build/               # Stale .o and .a artifacts
```

---

## Build Flow

```
stage1.asm (512B MBR, loaded by BIOS at 0x7C00)
    └── loads stage2.bin from disk (INT 13h, CHS)
        └── stage2.asm (real mode, boot menu)
            └── jumps to loader.asm (at 0x10000)
                └── enables A20, sets up GDT, enters protected mode
                    └── parses kernel ELF, loads to 0x100000
                        └── jumps to kernel_entry
                            └── kernel.arch.x86_64.boot.entry.asm
                                └── kernel_early_init (main.cpp)
                                    └── kernel_init (init.cpp)
                                        └── scheduler + shell
```

---

## Current State Summary

| Subsystem | Status |
|-----------|--------|
| **Bootloader (flat binary path)** | Working — stage1→stage2→loader→kernel chain complete. Kernel truncated (only 110/162 sectors loaded). |
| **Bootloader (ELF C++ path)** | NOT WORKING — 20+ duplicate symbols, broken include paths, never compiled. |
| **Kernel core** | Working — panic, printk, assertions, GDT/IDT/PIC/serial/console init. |
| **Scheduler** | Minimal — round-robin with linked-list run queue. Fork is stub. |
| **Syscalls** | 8 implemented (read, write, open, close, exit, getpid, getdents). 10 defined but unimplemented. No `copy_from_user`/`copy_to_user`. |
| **VFS** | Basic path resolution, open/read/write/readdir for ramfs. |
| **ramfs** | Working — in-memory root filesystem with create/mkdir/unlink/read/write. |
| **Memory management** | Simple bitmap PMM, bump heap allocator (kfree is no-op). Paging set up but VMM is stub. |
| **Devices** | Serial, PS/2 keyboard, PIT timer, CMOS RTC, PCI enumeration, ATA PIO. AHCI/NVMe/floppy are stubs. |
| **Graphics** | VGA text mode + basic framebuffer pixel ops. |
| **Userland** | Compiles into kernel. Init starts shell with cd/echo/exit/help/ls/cat builtins. |
| **Tests** | Nothing yet. |
| **Config system** | Nothing yet. |
| **ISO/initrd/UEFI** | Nothing yet. |
| **Networking** | Nothing yet. |

---

## Known Issues

This document describes the **actual** codebase. For a detailed list of bugs, design
problems, and missing features, see `ISSUES.md`.
