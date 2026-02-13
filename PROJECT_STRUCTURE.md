# Advanced C++ Operating System - Project Structure

This document outlines the complete structure for building an advanced OS from scratch with bootable ISO support for VirtualBox.

## Directory Structure

```
DIT-OS/
├── .github/
│   ├── workflows/
│   │   ├── build.yml              # CI/CD build workflow
│   │   ├── test.yml               # Test workflow
│   │   └── release.yml            # Release workflow
│   ├── ISSUE_TEMPLATE/
│   │   ├── bug_report.md
│   │   └── feature_request.md
│   └── PULL_REQUEST_TEMPLATE.md
│
├── .vscode/
│   ├── settings.json              # VS Code settings
│   ├── tasks.json                 # Build tasks
│   └── launch.json                # Debug configuration
│
├── bootloader/
│   ├── src/
│   │   ├── stage1/
│   │   │   ├── boot.asm           # Stage 1 bootloader (512 bytes)
│   │   │   ├── boot.hpp             # Boot sector header definitions
│   │   │   └── Makefile
│   │   ├── stage2/
│   │   │   ├── stage2.asm         # Stage 2 bootloader (real mode)
│   │   │   ├── stage2.cpp           # C wrapper for stage 2
│   │   │   ├── protected_mode.asm # Switch to protected mode
│   │   │   ├── long_mode.asm      # Switch to long mode
│   │   │   ├── a20.asm            # A20 line enable
│   │   │   ├── disk.cpp             # Disk I/O operations
│   │   │   ├── disk.asm           # Disk I/O assembly routines
│   │   │   ├── fs.cpp               # Simple filesystem reader (FAT/ext2)
│   │   │   ├── fat.cpp              # FAT12/16/32 reader
│   │   │   ├── ext2.cpp             # EXT2 reader
│   │   │   ├── iso9660.cpp          # ISO9660 reader
│   │   │   ├── memory.cpp           # Memory map detection
│   │   │   ├── memory.asm         # E820 memory map
│   │   │   ├── video.cpp            # Video mode setup
│   │   │   ├── video.asm          # VESA/VGA video modes
│   │   │   ├── console.cpp          # Boot console output
│   │   │   ├── elf.cpp              # ELF loader
│   │   │   ├── gdt.cpp              # Global Descriptor Table setup
│   │   │   ├── gdt.asm
│   │   │   ├── paging.cpp           # Early paging setup
│   │   │   └── Makefile
│   │   └── grub/
│   │       └── grub.cfg           # GRUB configuration (alternative bootloader)
│   ├── include/
│   │   ├── boot.hpp
│   │   ├── disk.hpp
│   │   ├── fs.hpp
│   │   ├── memory.hpp
│   │   ├── video.hpp
│   │   ├── elf.hpp
│   │   └── types.hpp
│   ├── scripts/
│   │   ├── build.sh
│   │   └── sign.sh                # Boot sector signing
│   └── build/
│
├── kernel/
│   ├── arch/
│   │   ├── x86_64/
│   │   │   ├── boot/
│   │   │   │   ├── multiboot.asm  # Multiboot header
│   │   │   │   ├── multiboot2.asm # Multiboot2 header
│   │   │   │   ├── entry.asm      # Kernel entry point
│   │   │   │   ├── init.cpp         # Early initialization
│   │   │   │   ├── main.cpp         # Kernel main
│   │   │   │   ├── trampoline.asm # SMP trampoline
│   │   │   │   └── linker.ld      # Linker script
│   │   │   ├── cpu/
│   │   │   │   ├── gdt.asm        # Global Descriptor Table
│   │   │   │   ├── gdt.cpp
│   │   │   │   ├── idt.asm        # Interrupt Descriptor Table
│   │   │   │   ├── idt.cpp
│   │   │   │   ├── tss.cpp          # Task State Segment
│   │   │   │   ├── tss.asm
│   │   │   │   ├── cpuid.cpp        # CPU feature detection
│   │   │   │   ├── msr.cpp          # Model Specific Registers
│   │   │   │   ├── msr.asm
│   │   │   │   ├── pic.cpp          # Programmable Interrupt Controller
│   │   │   │   ├── pic.asm
│   │   │   │   ├── apic.cpp         # Advanced PIC
│   │   │   │   ├── ioapic.cpp       # I/O APIC
│   │   │   │   ├── smp.cpp          # Symmetric Multi-Processing
│   │   │   │   ├── smp.asm
│   │   │   │   ├── lapic.cpp        # Local APIC
│   │   │   │   ├── cpuid.asm
│   │   │   │   ├── control.cpp      # Control registers
│   │   │   │   ├── control.asm
│   │   │   │   ├── mtrr.cpp         # Memory Type Range Registers
│   │   │   │   ├── pat.cpp          # Page Attribute Table
│   │   │   │   └── cpufeatures.cpp
│   │   │   ├── mm/
│   │   │   │   ├── paging.asm     # Page table management
│   │   │   │   ├── paging.cpp
│   │   │   │   ├── pmm.cpp          # Physical Memory Manager
│   │   │   │   ├── vmm.cpp          # Virtual Memory Manager
│   │   │   │   ├── heap.cpp         # Kernel heap allocator
│   │   │   │   ├── dma.cpp          # Direct Memory Access
│   │   │   │   ├── mtrr.asm
│   │   │   │   └── tlb.asm        # TLB management
│   │   │   ├── dev/
│   │   │   │   ├── serial.cpp       # Serial port (COM1/COM2)
│   │   │   │   ├── serial.hpp
│   │   │   │   ├── keyboard.cpp     # PS/2 keyboard driver
│   │   │   │   ├── keyboard.hpp
│   │   │   │   ├── mouse.cpp        # PS/2 mouse driver
│   │   │   │   ├── mouse.hpp
│   │   │   │   ├── timer.cpp        # PIT timer
│   │   │   │   ├── timer.hpp
│   │   │   │   ├── rtc.cpp          # Real-Time Clock
│   │   │   │   ├── rtc.hpp
│   │   │   │   ├── pci.cpp          # PCI bus enumeration
│   │   │   │   ├── pci_ids.hpp      # PCI device IDs
│   │   │   │   ├── ata.cpp          # ATA/IDE controller
│   │   │   │   ├── ata.hpp
│   │   │   │   ├── ahci.cpp         # AHCI/SATA controller
│   │   │   │   ├── ahci.hpp
│   │   │   │   ├── nvme.cpp         # NVMe controller
│   │   │   │   ├── nvme.hpp
│   │   │   │   ├── floppy.cpp       # Floppy disk driver
│   │   │   │   ├── usb/
│   │   │   │   │   ├── uhci.cpp     # USB UHCI controller
│   │   │   │   │   ├── ohci.cpp     # USB OHCI controller
│   │   │   │   │   ├── ehci.cpp     # USB EHCI controller
│   │   │   │   │   ├── xhci.cpp     # USB XHCI controller
│   │   │   │   │   ├── hub.cpp      # USB hub driver
│   │   │   │   │   ├── hid.cpp      # USB HID driver
│   │   │   │   │   └── mass_storage.cpp  # USB mass storage
│   │   │   │   └── network/
│   │   │   │       ├── e1000.cpp    # Intel E1000 NIC
│   │   │   │       ├── rtl8139.cpp  # Realtek RTL8139
│   │   │   │       └── virtio_net.cpp    # VirtIO network
│   │   │   ├── graphics/
│   │   │   │   ├── vga.cpp          # VGA text mode
│   │   │   │   ├── vga.hpp
│   │   │   │   ├── vbe.cpp          # VESA BIOS Extensions
│   │   │   │   ├── vbe.hpp
│   │   │   │   ├── framebuffer.cpp  # Linear framebuffer
│   │   │   │   ├── bochs_vbe.cpp    # Bochs VBE extensions
│   │   │   │   ├── fonts/
│   │   │   │   │   ├── font_8x16.cpp
│   │   │   │   │   ├── font_8x16.bin # Raw font data
│   │   │   │   │   ├── font_8x8.cpp
│   │   │   │   │   ├── font_8x8.bin
│   │   │   │   │   └── fonts.hpp
│   │   │   │   └── gpu/
│   │   │   │       ├── bochs_gpu.cpp
│   │   │   │       ├── vmware_svga.cpp
│   │   │   │       └── virtio_gpu.cpp
│   │   │   ├── debug/
│   │   │   │   ├── debug.cpp
│   │   │   │   ├── gdbstub.cpp      # GDB remote stub
│   │   │   │   ├── backtrace.cpp    # Stack unwinding
│   │   │   │   └── Makefile
│   │   │   └── Makefile
│   │   └── arm64/                 # Future ARM64 support
│   │       ├── boot/
│   │       │   └── linker.ld
│   │       ├── cpu/
│   │       ├── mm/
│   │       ├── dev/
│   │       └── Makefile
│   │
│   ├── core/
│   │   ├── kernel.cpp               # Kernel main entry
│   │   ├── panic.cpp                # Kernel panic handler
│   │   ├── panic.hpp
│   │   ├── printk.cpp               # Kernel printf implementation
│   │   ├── printk.hpp
│   │   ├── console.cpp              # Kernel console system
│   │   ├── console.hpp
│   │   ├── kassert.cpp              # Kernel assertions
│   │   ├── kassert.hpp
│   │   ├── cmdline.cpp              # Kernel command line parsing
│   │   ├── cmdline.hpp
│   │   ├── version.cpp              # Version information
│   │   ├── version.hpp
│   │   ├── logo.cpp                 # Kernel boot logo
│   │   ├── logo.hpp
│   │   ├── config.cpp               # Kernel configuration
│   │   ├── config.hpp
│   │   ├── modules.cpp              # Kernel module support
│   │   ├── modules.hpp
│   │   ├── symbols.cpp              # Kernel symbol table
│   │   ├── symbols.hpp
│   │   ├── compiler.hpp             # Compiler attributes
│   │   ├── compiler_gcc.hpp
│   │   ├── compiler_clang.hpp
│   │   └── Makefile
│   │
│   ├── include/
│   │   ├── kernel.hpp
│   │   ├── types.hpp
│   │   ├── stddef.hpp
│   │   ├── stdint.hpp
│   │   ├── stdbool.hpp
│   │   ├── string.hpp
│   │   ├── stdlib.hpp
│   │   ├── errno.hpp
│   │   ├── errno_defs.hpp           # Error code definitions
│   │   ├── limits.hpp
│   │   ├── ctype.hpp
│   │   ├── math.hpp
│   │   ├── assert.hpp
│   │   ├── kconfig.hpp              # Kernel configuration header
│   │   ├── kconfig_defaults.hpp     # Default config values
│   │   ├── config.hpp               # Auto-generated config
│   │   ├── config/
│   │   │   ├── debug.hpp
│   │   │   ├── mm.hpp
│   │   │   ├── sched.hpp
│   │   │   ├── fs.hpp
│   │   │   ├── net.hpp
│   │   │   └── drivers.hpp
│   │   ├── arch/
│   │   │   ├── x86_64/
│   │   │   │   ├── cpu.hpp
│   │   │   │   ├── gdt.hpp
│   │   │   │   ├── idt.hpp
│   │   │   │   ├── tss.hpp
│   │   │   │   ├── io.hpp
│   │   │   │   ├── paging.hpp
│   │   │   │   ├── msr.hpp
│   │   │   │   ├── cpuid.hpp
│   │   │   │   ├── control.hpp
│   │   │   │   ├── apic.hpp
│   │   │   │   ├── pic.hpp
│   │   │   │   ├── smp.hpp
│   │   │   │   ├── lapic.hpp
│   │   │   │   ├── ioapic.hpp
│   │   │   │   └── arch.hpp
│   │   │   └── arm64/
│   │   │       └── arch.hpp
│   │   ├── mm/
│   │   │   ├── pmm.hpp
│   │   │   ├── vmm.hpp
│   │   │   ├── kmalloc.hpp
│   │   │   ├── slab.hpp
│   │   │   ├── page.hpp
│   │   │   ├── page-flags.hpp
│   │   │   ├── page-defs.hpp
│   │   │   ├── memblock.hpp
│   │   │   ├── early_mm.hpp
│   │   │   └── mmu.hpp
│   │   ├── dev/
│   │   │   ├── device.hpp
│   │   │   ├── driver.hpp
│   │   │   ├── bus.hpp
│   │   │   ├── class.hpp
│   │   │   ├── serial.hpp
│   │   │   ├── keyboard.hpp
│   │   │   ├── keycodes.hpp
│   │   │   ├── timer.hpp
│   │   │   ├── rtc.hpp
│   │   │   ├── pci.hpp
│   │   │   ├── pci_ids.hpp
│   │   │   ├── ata.hpp
│   │   │   ├── usb.hpp
│   │   │   ├── usb_spec.hpp
│   │   │   ├── blockdev.hpp
│   │   │   └── chardev.hpp
│   │   ├── fs/
│   │   │   ├── vfs.hpp
│   │   │   ├── inode.hpp
│   │   │   ├── dentry.hpp
│   │   │   ├── superblock.hpp
│   │   │   ├── file.hpp
│   │   │   ├── mount.hpp
│   │   │   ├── fs.hpp
│   │   │   ├── namei.hpp
│   │   │   ├── stat.hpp
│   │   │   ├── fcntl.hpp
│   │   │   ├── ioctl.hpp
│   │   │   ├── dirent.hpp
│   │   │   ├── poll.hpp
│   │   │   ├── select.hpp
│   │   │   └── buffer.hpp
│   │   ├── graphics/
│   │   │   ├── vga.hpp
│   │   │   ├── framebuffer.hpp
│   │   │   ├── font.hpp
│   │   │   ├── color.hpp
│   │   │   ├── drawing.hpp
│   │   │   ├── cursor.hpp
│   │   │   ├── terminal.hpp
│   │   │   └── terminal-buffer.hpp
│   │   ├── net/
│   │   │   ├── socket.hpp
│   │   │   ├── packet.hpp
│   │   │   ├── ip.hpp
│   │   │   ├── tcp.hpp
│   │   │   ├── udp.hpp
│   │   │   ├── icmp.hpp
│   │   │   ├── arp.hpp
│   │   │   ├── ethernet.hpp
│   │   │   ├── checksum.hpp
│   │   │   ├── net.hpp
│   │   │   ├── netdevice.hpp
│   │   │   ├── skbuff.hpp
│   │   │   ├── sock.hpp
│   │   │   ├── proto.hpp
│   │   │   ├── route.hpp
│   │   │   ├── if_ether.hpp
│   │   │   ├── if.hpp
│   │   │   ├── in.hpp
│   │   │   ├── inet.hpp
│   │   │   └── netfilter.hpp
│   │   ├── syscall/
│   │   │   ├── syscall.hpp
│   │   │   ├── syscall_table.hpp
│   │   │   ├── syscall_numbers.hpp
│   │   │   ├── syscall_defs.hpp
│   │   │   └── syscall_trace.hpp
│   │   ├── sched/
│   │   │   ├── sched.hpp
│   │   │   ├── task.hpp
│   │   │   ├── process.hpp
│   │   │   ├── thread.hpp
│   │   │   ├── signal.hpp
│   │   │   ├── wait.hpp
│   │   │   ├── timer.hpp
│   │   │   └── cpumask.hpp
│   │   └── lib/
│   │       ├── list.hpp
│   │       ├── queue.hpp
│   │       ├── stack.hpp
│   │       ├── hash.hpp
│   │       ├── bitmap.hpp
│   │       ├── string.hpp
│   │       ├── spinlock.hpp
│   │       ├── mutex.hpp
│   │       ├── semaphore.hpp
│   │       ├── rwlock.hpp
│   │       ├── atomic.hpp
│   │       ├── refcount.hpp
│   │       ├── kref.hpp
│   │       ├── kobject.hpp
│   │       ├── kset.hpp
│   │       ├── klist.hpp
│   │       ├── hlist.hpp
│   │       ├── rbtree.hpp
│   │       ├── bsearch.hpp
│   │       ├── sort.hpp
│   │       ├── ctype.hpp
│   │       ├── div64.hpp
│   │       ├── crc32.hpp
│   │       ├── crc16.hpp
│   │       └── checksum.hpp
│   │
│   ├── mm/
│   │   ├── pmm.cpp                  # Physical memory management
│   │   ├── vmm.cpp                  # Virtual memory management
│   │   ├── kmalloc.cpp              # Kernel allocator (slab/buddy)
│   │   ├── slab.cpp                 # Slab allocator
│   │   ├── buddy.cpp                # Buddy allocator
│   │   ├── page_fault.cpp           # Page fault handler
│   │   ├── swap.cpp                 # Swap space management
│   │   ├── memblock.cpp             # Early memory allocator
│   │   ├── early_mm.cpp             # Early memory setup
│   │   ├── page_alloc.cpp           # Page allocation
│   │   ├── page-writeback.cpp       # Page writeback
│   │   ├── mmap.cpp                 # Memory mapping
│   │   ├── mremap.cpp               # Remap pages
│   │   ├── mlock.cpp                # Lock pages
│   │   ├── madvise.cpp              # Memory advice
│   │   ├── mincore.cpp              # Check page residency
│   │   ├── memory.cpp               # Memory info
│   │   ├── percpu.cpp               # Per-CPU allocations
│   │   ├── vmalloc.cpp              # vmalloc allocator
│   │   ├── ioremap.cpp              # I/O remapping
│   │   ├── highmem.cpp              # High memory handling
│   │   ├── page_io.cpp              # Page I/O
│   │   ├── truncate.cpp             # Truncate pages
│   │   ├── filemap.cpp              # File-backed mappings
│   │   ├── readahead.cpp            # Read-ahead
│   │   ├── swapfile.cpp             # Swap file support
│   │   ├── swap_state.cpp           # Swap state
│   │   ├── swap_slots.cpp           # Swap slot management
│   │   ├── page.cpp                 # Page operations
│   │   ├── page_alloc_ext.cpp       # Extended allocators
│   │   ├── memory_hotplug.cpp       # Hot-plug support
│   │   ├── memory.cpp
│   │   └── Makefile
│   │
│   ├── sched/
│   │   ├── sched.cpp                # Main scheduler
│   │   ├── task.cpp                 # Task/thread management
│   │   ├── fork.cpp                 # Process creation
│   │   ├── exec.cpp                 # Program execution
│   │   ├── exit.cpp                 # Process termination
│   │   ├── wait.cpp                 # Wait for child processes
│   │   ├── signal.cpp               # Signal handling
│   │   ├── signal.hpp               
│   │   ├── timer.cpp                # Timer/scheduling
│   │   ├── context.asm            # Context switching
│   │   ├── swtch.asm              # Context switch
│   │   ├── entry.asm              # Task entry points
│   │   ├── mutex.cpp                # Mutex implementation
│   │   ├── semaphore.cpp            # Semaphore implementation
│   │   ├── rwlock.cpp               # Read-write locks
│   │   ├── condvar.cpp              # Condition variables
│   │   ├── futex.cpp                # Fast userspace mutex
│   │   ├── process.cpp              # Process management
│   │   ├── thread.cpp               # Thread management
│   │   ├── pid.cpp                  # PID allocation
│   │   ├── loadavg.cpp              # Load average calculation
│   │   ├── clock.cpp                # Scheduling clock
│   │   ├── deadline.cpp             # Deadline scheduler
│   │   ├── fair.cpp                 # CFS scheduler
│   │   ├── rt.cpp                   # Real-time scheduler
│   │   ├── idle.cpp                 # Idle task
│   │   ├── stop_task.cpp            # Stop task
│   │   ├── cgroup.cpp               # Control groups
│   │   ├── namespace.cpp            # Namespaces
│   │   ├── core.cpp                 # Core scheduler
│   │   ├── autogroup.cpp            # Auto-grouping
│   │   ├── wait.cpp                 # Wait queues
│   │   ├── wait_bit.cpp             # Bit wait queues
│   │   ├── waitq.cpp                # Wait queue implementation
│   │   ├── completion.cpp           # Completion mechanism
│   │   ├── smp.cpp                  # SMP scheduling
│   │   ├── cpuidle.cpp              # CPU idle management
│   │   ├── cpufreq.cpp              # CPU frequency scaling
│   │   ├── sched.hpp                # Internal scheduler headers
│   │   ├── sched_api.hpp
│   │   └── Makefile
│   │
│   ├── fs/
│   │   ├── vfs.cpp                  # Virtual File System
│   │   ├── file.cpp                 # File operations
│   │   ├── inode.cpp                # Inode management
│   │   ├── dentry.cpp               # Directory entry cache
│   │   ├── superblock.cpp           # Filesystem superblocks
│   │   ├── mount.cpp                # Mount management
│   │   ├── path.cpp                 # Path resolution
│   │   ├── buffer.cpp               # Buffer cache
│   │   ├── namei.cpp                # Name resolution
│   │   ├── open.cpp                 # Open file handling
│   │   ├── read_write.cpp           # Read/write operations
│   │   ├── readdir.cpp              # Directory reading
│   │   ├── stat.cpp                 # File status
│   │   ├── ioctl.cpp                # I/O control
│   │   ├── fcntl.cpp                # File control
│   │   ├── pipe.cpp                 # Pipe operations
│   │   ├── fifo.cpp                 # Named pipes
│   │   ├── splice.cpp               # Splice operation
│   │   ├── sync.cpp                 # File synchronization
│   │   ├── fs-writeback.cpp         # Writeback management
│   │   ├── fsync.cpp                # fsync operations
│   │   ├── lock.cpp                 # File locking
│   │   ├── dcache.cpp               # Directory cache
│   │   ├── namespace.cpp            # Mount namespaces
│   │   ├── seq_file.cpp             # Sequential file I/O
│   │   ├── libfs.cpp                # FS library functions
│   │   ├── file_table.cpp           # File descriptor table
│   │   ├── mpage.cpp                # Multi-page I/O
│   │   ├── direct-io.cpp            # Direct I/O
│   │   ├── bad_inode.cpp            # Bad inode handling
│   │   ├── devfs/                 # Device filesystem
│   │   │   ├── devfs.cpp
│   │   │   ├── devfs.hpp
│   │   │   └── Makefile
│   │   ├── ramfs/                 # RAM filesystem
│   │   │   ├── ramfs.cpp
│   │   │   ├── ramfs.hpp
│   │   │   ├── inode.cpp
│   │   │   └── Makefile
│   │   ├── ext2/                  # EXT2 filesystem
│   │   │   ├── ext2.cpp
│   │   │   ├── ext2.hpp
│   │   │   ├── super.cpp
│   │   │   ├── inode.cpp
│   │   │   ├── file.cpp
│   │   │   ├── dir.cpp
│   │   │   ├── namei.cpp
│   │   │   ├── balloc.cpp           # Block allocation
│   │   │   ├── ialloc.cpp           # Inode allocation
│   │   │   ├── truncate.cpp
│   │   │   ├── symlink.cpp
│   │   │   ├── ioctl.cpp
│   │   │   └── Makefile
│   │   ├── fat32/                 # FAT32 filesystem
│   │   │   ├── fat32.cpp
│   │   │   ├── fat32.hpp
│   │   │   ├── cache.cpp
│   │   │   ├── dir.cpp
│   │   │   ├── file.cpp
│   │   │   ├── inode.cpp
│   │   │   ├── misc.cpp
│   │   │   ├── namei.cpp
│   │   │   └── Makefile
│   │   ├── iso9660/               # ISO 9660 (CD-ROM)
│   │   │   ├── iso9660.cpp
│   │   │   ├── iso9660.hpp
│   │   │   ├── name.cpp
│   │   │   ├── inode.cpp
│   │   │   ├── dir.cpp
│   │   │   └── Makefile
│   │   ├── tmpfs/                 # Temporary filesystem
│   │   │   ├── tmpfs.cpp
│   │   │   ├── tmpfs.hpp
│   │   │   └── Makefile
│   │   ├── procfs/                # Proc filesystem
│   │   │   ├── procfs.cpp
│   │   │   ├── procfs.hpp
│   │   │   ├── root.cpp
│   │   │   ├── meminfo.cpp
│   │   │   ├── cpuinfo.cpp
│   │   │   ├── version.cpp
│   │   │   ├── uptime.cpp
│   │   │   ├── stat.cpp
│   │   │   ├── modules.cpp
│   │   │   ├── mounts.cpp
│   │   │   └── Makefile
│   │   └── Makefile
│   │
│   ├── drivers/
│   │   ├── char/
│   │   │   ├── serial.cpp
│   │   │   ├── serial.hpp
│   │   │   ├── keyboard.cpp
│   │   │   ├── keyboard.hpp
│   │   │   ├── tty.cpp              # Terminal driver
│   │   │   ├── tty.hpp
│   │   │   ├── random.cpp           # Random number generator
│   │   │   ├── random.hpp
│   │   │   ├── urandom.cpp          # /dev/urandom
│   │   │   ├── mem.cpp              # /dev/mem, /dev/kmem
│   │   │   ├── null.cpp             # /dev/null
│   │   │   ├── zero.cpp             # /dev/zero
│   │   │   ├── full.cpp             # /dev/full
│   │   │   ├── port.cpp             # /dev/port
│   │   │   └── Makefile
│   │   ├── block/
│   │   │   ├── block.cpp            # Block layer
│   │   │   ├── block.hpp
│   │   │   ├── queue.cpp            # I/O request queue
│   │   │   ├── scheduler.cpp        # I/O scheduler
│   │   │   ├── elevator.cpp         # I/O elevator
│   │   │   ├── deadline-iosched.cpp # Deadline I/O scheduler
│   │   │   ├── cfq-iosched.cpp      # CFQ I/O scheduler
│   │   │   ├── noop-iosched.cpp     # No-op I/O scheduler
│   │   │   ├── ramdisk.cpp          # RAM disk driver
│   │   │   ├── loop.cpp             # Loopback device
│   │   │   ├── genhd.cpp            # Generic hard disk
│   │   │   └── Makefile
│   │   ├── bus/
│   │   │   ├── pci.cpp              # PCI bus driver
│   │   │   ├── pci.hpp
│   │   │   ├── pci_ids.cpp          # PCI device database
│   │   │   ├── pci_ids.hpp
│   │   │   ├── pci_sysfs.cpp        # PCI sysfs
│   │   │   ├── pci_probe.cpp        # PCI probing
│   │   │   ├── usb.cpp              # USB core driver
│   │   │   ├── usb.hpp
│   │   │   ├── usb_hub.cpp
│   │   │   ├── usb_device.cpp
│   │   │   ├── usb_hcd.cpp          # Host controller driver
│   │   │   └── Makefile
│   │   ├── storage/
│   │   │   ├── ata.cpp
│   │   │   ├── ata.hpp
│   │   │   ├── libata.cpp           # ATA library
│   │   │   ├── ahci.cpp
│   │   │   ├── ahci.hpp
│   │   │   ├── nvme.cpp
│   │   │   ├── nvme.hpp
│   │   │   ├── scsi.cpp             # SCSI support
│   │   │   ├── scsi.hpp
│   │   │   ├── scsi_scan.cpp
│   │   │   ├── scsi_lib.cpp
│   │   │   ├── scsi_sysfs.cpp
│   │   │   └── Makefile
│   │   ├── input/
│   │   │   ├── keyboard.cpp
│   │   │   ├── mouse.cpp
│   │   │   ├── input.cpp            # Input core
│   │   │   ├── input.hpp
│   │   │   ├── event.cpp            # Input event system
│   │   │   ├── evdev.cpp            # Event device
│   │   │   ├── serio.cpp            # Serial I/O
│   │   │   └── Makefile
│   │   ├── graphics/
│   │   │   ├── vga.cpp
│   │   │   ├── vga.hpp
│   │   │   ├── vbe.cpp
│   │   │   ├── vbe.hpp
│   │   │   ├── fb.cpp               # Framebuffer
│   │   │   ├── fb.hpp
│   │   │   ├── fbmem.cpp            # Framebuffer memory
│   │   │   ├── fbcmap.cpp           # Framebuffer colormap
│   │   │   ├── fbsysfs.cpp          # Framebuffer sysfs
│   │   │   └── drm/               # Direct Rendering Manager
│   │   │       ├── drm.cpp
│   │   │       ├── drm.hpp
│   │   │       └── Makefile
│   │   ├── net/
│   │   │   ├── network.cpp          # Network stack core
│   │   │   ├── network.hpp
│   │   │   ├── arp.cpp
│   │   │   ├── ip.cpp
│   │   │   ├── ip_input.cpp
│   │   │   ├── ip_output.cpp
│   │   │   ├── ip_fragment.cpp
│   │   │   ├── ip_options.cpp
│   │   │   ├── icmp.cpp
│   │   │   ├── tcp.cpp
│   │   │   ├── tcp_input.cpp
│   │   │   ├── tcp_output.cpp
│   │   │   ├── tcp_timer.cpp
│   │   │   ├── tcp_cong.cpp         # TCP congestion control
│   │   │   ├── udp.cpp
│   │   │   ├── socket.cpp
│   │   │   ├── socket.hpp
│   │   │   ├── af_inet.cpp          # INET address family
│   │   │   ├── netdev.cpp           # Network devices
│   │   │   ├── netdev.hpp
│   │   │   ├── packet.cpp           # Packet buffer
│   │   │   ├── route.cpp            # Routing table
│   │   │   ├── fib.cpp              # Forwarding Information Base
│   │   │   ├── checksum.cpp
│   │   │   ├── ethernet.cpp
│   │   │   ├── etherdevice.cpp
│   │   │   └── Makefile
│   │   └── Makefile
│   │
│   ├── syscall/
│   │   ├── syscall.cpp              # System call dispatcher
│   │   ├── syscall.hpp
│   │   ├── syscall_table.cpp        # System call table
│   │   ├── syscall_table.hpp
│   │   ├── entry.asm              # System call entry
│   │   ├── sys_exit.cpp
│   │   ├── sys_fork.cpp
│   │   ├── sys_exec.cpp
│   │   ├── sys_wait.cpp
│   │   ├── sys_open.cpp
│   │   ├── sys_read.cpp
│   │   ├── sys_write.cpp
│   │   ├── sys_close.cpp
│   │   ├── sys_mmap.cpp
│   │   ├── sys_munmap.cpp
│   │   ├── sys_ioctl.cpp
│   │   ├── sys_stat.cpp
│   │   ├── sys_fstat.cpp
│   │   ├── sys_lstat.cpp
│   │   ├── sys_access.cpp
│   │   ├── sys_chmod.cpp
│   │   ├── sys_chown.cpp
│   │   ├── sys_link.cpp
│   │   ├── sys_unlink.cpp
│   │   ├── sys_rename.cpp
│   │   ├── sys_mkdir.cpp
│   │   ├── sys_rmdir.cpp
│   │   ├── sys_dup.cpp
│   │   ├── sys_dup2.cpp
│   │   ├── sys_pipe.cpp
│   │   ├── sys_getpid.cpp
│   │   ├── sys_getppid.cpp
│   │   ├── sys_getuid.cpp
│   │   ├── sys_setuid.cpp
│   │   ├── sys_getgid.cpp
│   │   ├── sys_setgid.cpp
│   │   ├── sys_geteuid.cpp
│   │   ├── sys_getegid.cpp
│   │   ├── sys_kill.cpp
│   │   ├── sys_signal.cpp
│   │   ├── sys_sigaction.cpp
│   │   ├── sys_sigsuspend.cpp
│   │   ├── sys_sigpending.cpp
│   │   ├── sys_alarm.cpp
│   │   ├── sys_nanosleep.cpp
│   │   ├── sys_gettimeofday.cpp
│   │   ├── sys_settimeofday.cpp
│   │   ├── sys_times.cpp
│   │   ├── sys_brk.cpp
│   │   ├── sys_sbrk.cpp
│   │   ├── sys_clone.cpp
│   │   ├── sys_mount.cpp
│   │   ├── sys_umount.cpp
│   │   ├── sys_sync.cpp
│   │   ├── sys_reboot.cpp
│   │   ├── sys_sysinfo.cpp
│   │   ├── sys_uname.cpp
│   │   ├── sys_getcwd.cpp
│   │   ├── sys_chdir.cpp
│   │   ├── sys_fchdir.cpp
│   │   ├── sys_readlink.cpp
│   │   ├── sys_getdents.cpp
│   │   ├── sys_lseek.cpp
│   │   ├── sys_fcntl.cpp
│   │   ├── sys_select.cpp
│   │   ├── sys_poll.cpp
│   │   ├── sys_epoll.cpp
│   │   ├── sys_socket.cpp
│   │   ├── sys_bind.cpp
│   │   ├── sys_listen.cpp
│   │   ├── sys_accept.cpp
│   │   ├── sys_connect.cpp
│   │   ├── sys_sendto.cpp
│   │   ├── sys_recvfrom.cpp
│   │   ├── sys_shutdown.cpp
│   │   ├── sys_setsockopt.cpp
│   │   ├── sys_getsockopt.cpp
│   │   ├── sys_mprotect.cpp
│   │   ├── sys_msync.cpp
│   │   ├── sys_mlock.cpp
│   │   ├── sys_munlock.cpp
│   │   ├── sys_mlockall.cpp
│   │   ├── sys_munlockall.cpp
│   │   ├── sys_ptrace.cpp
│   │   ├── sys_ioctl.cpp
│   │   ├── sys_fsync.cpp
│   │   ├── sys_fdatasync.cpp
│   │   ├── sys_truncate.cpp
│   │   ├── sys_ftruncate.cpp
│   │   ├── sys_umask.cpp
│   │   ├── sys_chroot.cpp
│   │   ├── sys_mknod.cpp
│   │   ├── sys_symlink.cpp
│   │   ├── sys_lseek64.cpp
│   │   ├── sys_pread.cpp
│   │   ├── sys_pwrite.cpp
│   │   ├── sys_getrlimit.cpp
│   │   ├── sys_setrlimit.cpp
│   │   ├── sys_getrusage.cpp
│   │   ├── sys_syslog.cpp
│   │   ├── sys_ioperm.cpp
│   │   ├── sys_iopl.cpp
│   │   └── Makefile
│   │
│   ├── lib/
│   │   ├── string.cpp               # String operations
│   │   ├── string.hpp
│   │   ├── memory.cpp               # Memory operations
│   │   ├── memory.hpp
│   │   ├── vsprintf.cpp             # Format printing
│   │   ├── vsprintf.hpp
│   │   ├── ctype.cpp                # Character classification
│   │   ├── ctype.hpp
│   │   ├── list.cpp                 # Linked lists
│   │   ├── queue.cpp                # Queues
│   │   ├── queue.hpp
│   │   ├── hash.cpp                 # Hash tables
│   │   ├── hash.hpp
│   │   ├── bitmap.cpp               # Bitmap operations
│   │   ├── bitmap.hpp
│   │   ├── rbtree.cpp               # Red-black trees
│   │   ├── rbtree.hpp
│   │   ├── sort.cpp                 # Sorting algorithms
│   │   ├── sort.hpp
│   │   ├── math.cpp                 # Math functions
│   │   ├── math.hpp
│   │   ├── rand.cpp                 # Random number generation
│   │   ├── rand.hpp
│   │   ├── time.cpp                 # Time utilities
│   │   ├── time.hpp
│   │   ├── uuid.cpp                 # UUID generation
│   │   ├── uuid.hpp
│   │   ├── base64.cpp               # Base64 encoding
│   │   ├── base64.hpp
│   │   ├── printf.cpp               # printf implementation
│   │   ├── kstrtox.cpp              # String to number conversion
│   │   ├── kstrtox.hpp
│   │   ├── hexdump.cpp              # Hex dump utility
│   │   ├── hexdump.hpp
│   │   ├── scatterlist.cpp          # Scatter-gather list
│   │   ├── scatterlist.hpp
│   │   ├── checksum.cpp             # Checksum utilities
│   │   ├── checksum.hpp
│   │   ├── crc32.cpp                # CRC32 implementation
│   │   ├── crc32.hpp
│   │   ├── crc16.cpp                # CRC16 implementation
│   │   ├── crc16.hpp
│   │   ├── crc7.cpp                 # CRC7 implementation
│   │   ├── crc7.hpp
│   │   ├── crc-ccitt.cpp            # CRC-CCITT
│   │   ├── bsearch.cpp              # Binary search
│   │   ├── bsearch.hpp
│   │   ├── llist.cpp                # Lock-less lists
│   │   ├── llist.hpp
│   │   ├── kfifo.cpp                # Kernel FIFO
│   │   ├── kfifo.hpp
│   │   ├── percpu-refcount.cpp      # Per-CPU reference counting
│   │   ├── percpu-refcount.hpp
│   │   ├── siphash.cpp              # SipHash
│   │   ├── siphash.hpp
│   │   ├── chacha.cpp               # ChaCha
│   │   ├── chacha.hpp
│   │   └── Makefile
│   │
│   └── Makefile
│
├── userland/
│   ├── libc/
│   │   ├── include/
│   │   │   ├── stdio.hpp
│   │   │   ├── stdlib.hpp
│   │   │   ├── string.hpp
│   │   │   ├── unistd.hpp
│   │   │   ├── fcntl.hpp
│   │   │   ├── dirent.hpp
│   │   │   ├── ctype.hpp
│   │   │   ├── time.hpp
│   │   │   ├── signal.hpp
│   │   │   ├── setjmp.hpp
│   │   │   ├── stdint.hpp
│   │   │   ├── stddef.hpp
│   │   │   ├── stdbool.hpp
│   │   │   ├── limits.hpp
│   │   │   ├── float.hpp
│   │   │   ├── math.hpp
│   │   │   ├── assert.hpp
│   │   │   ├── errno.hpp
│   │   │   ├── pwd.hpp
│   │   │   ├── grp.hpp
│   │   │   ├── utsname.hpp
│   │   │   ├── syslog.hpp
│   │   │   ├── termios.hpp
│   │   │   ├── ioctl.hpp
│   │   │   ├── poll.hpp
│   │   │   ├── select.hpp
│   │   │   ├── sys/
│   │   │   │   ├── types.hpp
│   │   │   │   ├── stat.hpp
│   │   │   │   ├── statfs.hpp
│   │   │   │   ├── wait.hpp
│   │   │   │   ├── mman.hpp
│   │   │   │   ├── syscall.hpp
│   │   │   │   ├── time.hpp
│   │   │   │   ├── resource.hpp
│   │   │   │   ├── uio.hpp
│   │   │   │   ├── socket.hpp
│   │   │   │   ├── un.hpp
│   │   │   │   ├── ioctl.hpp
│   │   │   │   └── reboot.hpp
│   │   │   └── bits/
│   │   │       ├── types.hpp
│   │   │       ├── stat.hpp
│   │   │       └── fcntl.hpp
│   │   ├── src/
│   │   │   ├── stdio/
│   │   │   │   ├── fopen.cpp
│   │   │   │   ├── fclose.cpp
│   │   │   │   ├── fread.cpp
│   │   │   │   ├── fwrite.cpp
│   │   │   │   ├── fgetc.cpp
│   │   │   │   ├── fputc.cpp
│   │   │   │   ├── fgets.cpp
│   │   │   │   ├── fputs.cpp
│   │   │   │   ├── fprintf.cpp
│   │   │   │   ├── fscanf.cpp
│   │   │   │   ├── printf.cpp
│   │   │   │   ├── scanf.cpp
│   │   │   │   ├── sprintf.cpp
│   │   │   │   ├── sscanf.cpp
│   │   │   │   ├── snprintf.cpp
│   │   │   │   ├── vprintf.cpp
│   │   │   │   ├── vfprintf.cpp
│   │   │   │   ├── vsprintf.cpp
│   │   │   │   ├── vsnprintf.cpp
│   │   │   │   ├── perror.cpp
│   │   │   │   ├── fflush.cpp
│   │   │   │   ├── fseek.cpp
│   │   │   │   ├── ftell.cpp
│   │   │   │   ├── rewind.cpp
│   │   │   │   ├── setbuf.cpp
│   │   │   │   ├── setvbuf.cpp
│   │   │   │   ├── getc.cpp
│   │   │   │   ├── putc.cpp
│   │   │   │   ├── getchar.cpp
│   │   │   │   ├── putchar.cpp
│   │   │   │   ├── gets.cpp
│   │   │   │   ├── puts.cpp
│   │   │   │   ├── ungetc.cpp
│   │   │   │   ├── feof.cpp
│   │   │   │   ├── ferror.cpp
│   │   │   │   ├── clearerr.cpp
│   │   │   │   ├── fileno.cpp
│   │   │   │   ├── tmpfile.cpp
│   │   │   │   ├── tmpnam.cpp
│   │   │   │   ├── remove.cpp
│   │   │   │   ├── rename.cpp
│   │   │   │   └── Makefile
│   │   │   ├── stdlib/
│   │   │   │   ├── malloc.cpp
│   │   │   │   ├── calloc.cpp
│   │   │   │   ├── realloc.cpp
│   │   │   │   ├── free.cpp
│   │   │   │   ├── abort.cpp
│   │   │   │   ├── exit.cpp
│   │   │   │   ├── atexit.cpp
│   │   │   │   ├── system.cpp
│   │   │   │   ├── getenv.cpp
│   │   │   │   ├── putenv.cpp
│   │   │   │   ├── setenv.cpp
│   │   │   │   ├── unsetenv.cpp
│   │   │   │   ├── clearenv.cpp
│   │   │   │   ├── abs.cpp
│   │   │   │   ├── labs.cpp
│   │   │   │   ├── llabs.cpp
│   │   │   │   ├── div.cpp
│   │   │   │   ├── ldiv.cpp
│   │   │   │   ├── lldiv.cpp
│   │   │   │   ├── atoi.cpp
│   │   │   │   ├── atol.cpp
│   │   │   │   ├── atoll.cpp
│   │   │   │   ├── strtol.cpp
│   │   │   │   ├── strtoll.cpp
│   │   │   │   ├── strtoul.cpp
│   │   │   │   ├── strtoull.cpp
│   │   │   │   ├── strtof.cpp
│   │   │   │   ├── strtod.cpp
│   │   │   │   ├── strtold.cpp
│   │   │   │   ├── rand.cpp
│   │   │   │   ├── srand.cpp
│   │   │   │   ├── rand_r.cpp
│   │   │   │   ├── qsort.cpp
│   │   │   │   ├── bsearch.cpp
│   │   │   │   ├── mbtowc.cpp
│   │   │   │   ├── wctomb.cpp
│   │   │   │   ├── mbstowcs.cpp
│   │   │   │   ├── wcstombs.cpp
│   │   │   │   ├── mblen.cpp
│   │   │   │   ├── realpath.cpp
│   │   │   │   ├── mkstemp.cpp
│   │   │   │   ├── mktemp.cpp
│   │   │   │   └── Makefile
│   │   │   ├── string/
│   │   │   │   ├── memcpy.cpp
│   │   │   │   ├── memmove.cpp
│   │   │   │   ├── memset.cpp
│   │   │   │   ├── memcmp.cpp
│   │   │   │   ├── memchr.cpp
│   │   │   │   ├── strcpy.cpp
│   │   │   │   ├── strncpy.cpp
│   │   │   │   ├── strcat.cpp
│   │   │   │   ├── strncat.cpp
│   │   │   │   ├── strcmp.cpp
│   │   │   │   ├── strncmp.cpp
│   │   │   │   ├── strlen.cpp
│   │   │   │   ├── strchr.cpp
│   │   │   │   ├── strrchr.cpp
│   │   │   │   ├── strstr.cpp
│   │   │   │   ├── strpbrk.cpp
│   │   │   │   ├── strspn.cpp
│   │   │   │   ├── strcspn.cpp
│   │   │   │   ├── strtok.cpp
│   │   │   │   ├── strtok_r.cpp
│   │   │   │   ├── strerror.cpp
│   │   │   │   ├── strerror_r.cpp
│   │   │   │   ├── strdup.cpp
│   │   │   │   ├── strndup.cpp
│   │   │   │   ├── strcasecmp.cpp
│   │   │   │   ├── strncasecmp.cpp
│   │   │   │   ├── strcoll.cpp
│   │   │   │   ├── strxfrm.cpp
│   │   │   │   └── Makefile
│   │   │   ├── unistd/
│   │   │   │   ├── access.cpp
│   │   │   │   ├── alarm.cpp
│   │   │   │   ├── brk.cpp
│   │   │   │   ├── chdir.cpp
│   │   │   │   ├── chown.cpp
│   │   │   │   ├── close.cpp
│   │   │   │   ├── dup.cpp
│   │   │   │   ├── dup2.cpp
│   │   │   │   ├── execl.cpp
│   │   │   │   ├── execv.cpp
│   │   │   │   ├── execve.cpp
│   │   │   │   ├── execvp.cpp
│   │   │   │   ├── _exit.cpp
│   │   │   │   ├── fork.cpp
│   │   │   │   ├── getcwd.cpp
│   │   │   │   ├── getegid.cpp
│   │   │   │   ├── geteuid.cpp
│   │   │   │   ├── getgid.cpp
│   │   │   │   ├── getgroups.cpp
│   │   │   │   ├── getlogin.cpp
│   │   │   │   ├── getpgid.cpp
│   │   │   │   ├── getpgrp.cpp
│   │   │   │   ├── getpid.cpp
│   │   │   │   ├── getppid.cpp
│   │   │   │   ├── getuid.cpp
│   │   │   │   ├── isatty.cpp
│   │   │   │   ├── link.cpp
│   │   │   │   ├── lseek.cpp
│   │   │   │   ├── pipe.cpp
│   │   │   │   ├── read.cpp
│   │   │   │   ├── rmdir.cpp
│   │   │   │   ├── setgid.cpp
│   │   │   │   ├── setpgid.cpp
│   │   │   │   ├── setsid.cpp
│   │   │   │   ├── setuid.cpp
│   │   │   │   ├── sleep.cpp
│   │   │   │   ├── symlink.cpp
│   │   │   │   ├── sysconf.cpp
│   │   │   │   ├── tcgetpgrp.cpp
│   │   │   │   ├── tcsetpgrp.cpp
│   │   │   │   ├── ttyname.cpp
│   │   │   │   ├── ualarm.cpp
│   │   │   │   ├── unlink.cpp
│   │   │   │   ├── usleep.cpp
│   │   │   │   ├── write.cpp
│   │   │   │   └── Makefile
│   │   │   ├── syscall/
│   │   │   │   ├── syscall.cpp      # Syscall wrapper
│   │   │   │   ├── syscall.S      # Assembly syscall entry
│   │   │   │   └── Makefile
│   │   │   ├── crt0.asm           # C runtime startup
│   │   │   ├── ctype/
│   │   │   │   ├── isalnum.cpp
│   │   │   │   ├── isalpha.cpp
│   │   │   │   ├── isascii.cpp
│   │   │   │   ├── isblank.cpp
│   │   │   │   ├── iscntrl.cpp
│   │   │   │   ├── isdigit.cpp
│   │   │   │   ├── isgraph.cpp
│   │   │   │   ├── islower.cpp
│   │   │   │   ├── isprint.cpp
│   │   │   │   ├── ispunct.cpp
│   │   │   │   ├── isspace.cpp
│   │   │   │   ├── isupper.cpp
│   │   │   │   ├── isxdigit.cpp
│   │   │   │   ├── tolower.cpp
│   │   │   │   ├── toupper.cpp
│   │   │   │   └── Makefile
│   │   │   └── Makefile
│   │   ├── lib/                   # Static library
│   │   │   ├── libc.a
│   │   └── Makefile
│   │
│   ├── shell/
│   │   ├── src/
│   │   │   ├── main.cpp
│   │   │   ├── parser.cpp
│   │   │   ├── parser.hpp
│   │   │   ├── lexer.cpp
│   │   │   ├── lexer.hpp
│   │   │   ├── builtin.cpp          # Built-in commands
│   │   │   ├── builtin.hpp
│   │   │   ├── exec.cpp
│   │   │   ├── exec.hpp
│   │   │   ├── job.cpp              # Job control
│   │   │   ├── job.hpp
│   │   │   ├── env.cpp              # Environment variables
│   │   │   ├── env.hpp
│   │   │   ├── history.cpp          # Command history
│   │   │   ├── history.hpp
│   │   │   ├── prompt.cpp           # Shell prompt
│   │   │   ├── prompt.hpp
│   │   │   ├── signal.cpp           # Signal handling
│   │   │   ├── signal.hpp
│   │   │   ├── redirection.cpp      # I/O redirection
│   │   │   ├── redirection.hpp
│   │   │   ├── pipe.cpp             # Pipe handling
│   │   │   ├── pipe.hpp
│   │   │   ├── wildcard.cpp         # Glob patterns
│   │   │   ├── wildcard.hpp
│   │   │   ├── variables.cpp        # Shell variables
│   │   │   ├── variables.hpp
│   │   │   ├── complete.cpp         # Tab completion
│   │   │   ├── complete.hpp
│   │   │   └── Makefile
│   │   ├── include/
│   │   │   └── shell.hpp
│   │   └── Makefile
│   │
│   ├── bin/                       # User binaries
│   │   ├── init/                  # Init process
│   │   │   ├── init.cpp
│   │   │   └── Makefile
│   │   ├── ls/
│   │   │   ├── ls.cpp
│   │   │   └── Makefile
│   │   ├── cat/
│   │   │   ├── cat.cpp
│   │   │   └── Makefile
│   │   ├── echo/
│   │   │   ├── echo.cpp
│   │   │   └── Makefile
│   │   ├── mkdir/
│   │   │   ├── mkdir.cpp
│   │   │   └── Makefile
│   │   ├── rm/
│   │   │   ├── rm.cpp
│   │   │   └── Makefile
│   │   ├── cp/
│   │   │   ├── cp.cpp
│   │   │   └── Makefile
│   │   ├── mv/
│   │   │   ├── mv.cpp
│   │   │   └── Makefile
│   │   ├── ps/
│   │   │   ├── ps.cpp
│   │   │   └── Makefile
│   │   ├── kill/
│   │   │   ├── kill.cpp
│   │   │   └── Makefile
│   │   ├── clear/
│   │   │   ├── clear.cpp
│   │   │   └── Makefile
│   │   ├── date/
│   │   │   ├── date.cpp
│   │   │   └── Makefile
│   │   ├── pwd/
│   │   │   ├── pwd.cpp
│   │   │   └── Makefile
│   │   ├── cd/                    # Built into shell
│   │   │   └── Makefile
│   │   ├── mount/
│   │   │   ├── mount.cpp
│   │   │   └── Makefile
│   │   ├── umount/
│   │   │   ├── umount.cpp
│   │   │   └── Makefile
│   │   ├── df/
│   │   │   ├── df.cpp
│   │   │   └── Makefile
│   │   ├── du/
│   │   │   ├── du.cpp
│   │   │   └── Makefile
│   │   ├── grep/
│   │   │   ├── grep.cpp
│   │   │   └── Makefile
│   │   ├── test/
│   │   │   ├── test.cpp
│   │   │   └── Makefile
│   │   ├── chmod/
│   │   │   ├── chmod.cpp
│   │   │   └── Makefile
│   │   ├── chown/
│   │   │   ├── chown.cpp
│   │   │   └── Makefile
│   │   ├── ln/
│   │   │   ├── ln.cpp
│   │   │   └── Makefile
│   │   ├── touch/
│   │   │   ├── touch.cpp
│   │   │   └── Makefile
│   │   ├── head/
│   │   │   ├── head.cpp
│   │   │   └── Makefile
│   │   ├── tail/
│   │   │   ├── tail.cpp
│   │   │   └── Makefile
│   │   ├── wc/
│   │   │   ├── wc.cpp
│   │   │   └── Makefile
│   │   ├── sort/
│   │   │   ├── sort.cpp
│   │   │   └── Makefile
│   │   ├── uniq/
│   │   │   ├── uniq.cpp
│   │   │   └── Makefile
│   │   ├── tr/
│   │   │   ├── tr.cpp
│   │   │   └── Makefile
│   │   ├── tee/
│   │   │   ├── tee.cpp
│   │   │   └── Makefile
│   │   ├── more/
│   │   │   ├── more.cpp
│   │   │   └── Makefile
│   │   ├── less/
│   │   │   ├── less.cpp
│   │   │   └── Makefile
│   │   ├── find/
│   │   │   ├── find.cpp
│   │   │   └── Makefile
│   │   ├── xargs/
│   │   │   ├── xargs.cpp
│   │   │   └── Makefile
│   │   ├── whoami/
│   │   │   ├── whoami.cpp
│   │   │   └── Makefile
│   │   ├── id/
│   │   │   ├── id.cpp
│   │   │   └── Makefile
│   │   ├── hostname/
│   │   │   ├── hostname.cpp
│   │   │   └── Makefile
│   │   ├── uname/
│   │   │   ├── uname.cpp
│   │   │   └── Makefile
│   │   ├── uptime/
│   │   │   ├── uptime.cpp
│   │   │   └── Makefile
│   │   ├── sleep/
│   │   │   ├── sleep.cpp
│   │   │   └── Makefile
│   │   ├── sync/
│   │   │   ├── sync.cpp
│   │   │   └── Makefile
│   │   ├── reboot/
│   │   │   ├── reboot.cpp
│   │   │   └── Makefile
│   │   └── Makefile
│   │
│   ├── sbin/
│   │   ├── login/
│   │   │   ├── login.cpp
│   │   │   └── Makefile
│   │   ├── getty/
│   │   │   ├── getty.cpp
│   │   │   └── Makefile
│   │   ├── fdisk/
│   │   │   ├── fdisk.cpp
│   │   │   └── Makefile
│   │   ├── mkfs/
│   │   │   ├── mkfs.cpp
│   │   │   ├── mkfs_ext2.cpp
│   │   │   ├── mkfs_fat.cpp
│   │   │   └── Makefile
│   │   ├── fsck/
│   │   │   ├── fsck.cpp
│   │   │   ├── fsck_ext2.cpp
│   │   │   └── Makefile
│   │   ├── ifconfig/
│   │   │   ├── ifconfig.cpp
│   │   │   └── Makefile
│   │   ├── route/
│   │   │   ├── route.cpp
│   │   │   └── Makefile
│   │   ├── ping/
│   │   │   ├── ping.cpp
│   │   │   └── Makefile
│   │   ├── dhcp/
│   │   │   ├── dhcp.cpp
│   │   │   └── Makefile
│   │   └── Makefile
│   │
│   ├── lib/                       # Shared libraries
│   │   ├── ld.so                  # Dynamic linker
│   │   ├── libc.so
│   │   ├── libm.so
│   │   └── Makefile
│   │
│   ├── etc/
│   │   ├── passwd
│   │   ├── shadow
│   │   ├── group
│   │   ├── motd
│   │   ├── fstab
│   │   ├── hostname
│   │   ├── hosts
│   │   ├── resolv.conf
│   │   ├── inittab
│   │   ├── profile
│   │   ├── shells
│   │   ├── termcap
│   │   ├── timezone
│   │   ├── services
│   │   ├── protocols
│   │   ├── networks
│   │   └── Makefile
│   │
│   ├── share/
│   │   ├── doc/
│   │   ├── man/
│   │   ├── locale/
│   │   └── Makefile
│   │
│   └── Makefile
│
├── initrd/                        # Initial RAM disk
│   ├── init                        # Init script
│   ├── bin/                        # Essential binaries
│   │   ├── sh -> /bin/bash        # Shell
│   │   ├── bash
│   │   ├── ls
│   │   ├── cat
│   │   ├── echo
│   │   ├── mkdir
│   │   ├── rm
│   │   ├── cp
│   │   ├── mv
│   │   ├── mount
│   │   ├── umount
│   │   ├── chmod
│   │   ├── chown
│   │   ├── ln
│   │   ├── touch
│   │   ├── ps
│   │   ├── kill
│   │   ├── clear
│   │   ├── pwd
│   │   ├── sleep
│   │   └── sync
│   ├── sbin/                       # System binaries
│   │   ├── switch_root
│   │   └── modprobe
│   ├── lib/                        # Shared libraries
│   │   ├── ld.so
│   │   └── libc.so
│   ├── etc/                        # Configuration
│   │   ├── inittab
│   │   ├── fstab.initrd
│   │   └── profile
│   ├── dev/                        # Device files (created at runtime)
│   ├── proc/                       # Proc mount point
│   ├── sys/                        # Sys mount point
│   ├── tmp/                        # Temp directory
│   └── Makefile
│
├── boot/
│   ├── grub/
│   │   ├── grub.cfg               # GRUB configuration
│   │   ├── grubenv
│   │   └── themes/
│
├── tools/
│   ├── cross-compiler/            # GCC cross-compiler build scripts
│   │   ├── build-binutils.sh
│   │   ├── build-gcc.sh
│   │   ├── build-gdb.sh
│   │   ├── download.sh
│   │   ├── config.sh
│   │   ├── patches/
│   │   └── README.md
│   ├── build-iso/                 # ISO creation utilities
│   │   ├── mkiso.sh
│   │   ├── mkiso.py
│   │   ├── isohybrid.cpp            # Make ISO bootable from USB
│   │   └── Makefile
│   ├── mkinitrd/                  # Initrd creation tool
│   │   ├── mkinitrd.cpp
│   │   ├── mkinitrd.sh
│   │   ├── cpio.cpp                 # CPIO archive format
│   │   └── Makefile
│   ├── font-converter/            # Font conversion utilities
│   │   ├── fontconv.cpp
│   │   ├── psf2c.cpp                # PSF to C array
│   │   ├── bdf2c.cpp                # BDF to C array
│   │   └── Makefile
│   ├── debug/
│   │   ├── gdb-init               # GDB configuration
│   │   ├── bochsrc                # Bochs emulator config
│   │   ├── bochs-debugger.sh
│   │   ├── qemu-gdb.sh
│   │   ├── trace.py               # Kernel tracing script
│   │   └── README.md
│   ├── kconfig/                   # Kernel configuration tool
│   │   ├── mconf.cpp                # Menuconfig
│   │   ├── conf.cpp                 # Config
│   │   ├── qconf.cpp                # Qt config (optional)
│   │   ├── gconf.cpp                # GTK config (optional)
│   │   └── Makefile
│   └── Makefile
│
├── docs/
│   ├── architecture/
│   │   ├── overview.md
│   │   ├── boot-process.md
│   │   ├── memory-management.md
│   │   ├── process-scheduling.md
│   │   ├── filesystem.md
│   │   ├── networking.md
│   │   ├── drivers.md
│   │   └── smp.md
│   ├── api/
│   │   ├── kernel-api.md
│   │   ├── syscalls.md
│   │   ├── vfs.md
│   │   └── driver-api.md
│   ├── drivers/
│   │   ├── writing-drivers.md
│   │   ├── pci.md
│   │   ├── usb.md
│   │   └── network.md
│   ├── boot/
│   │   ├── bootloader.md
│   │   ├── multiboot.md
│   │   └── grub-setup.md
│   ├── porting/
│   │   ├── porting-guide.md
│   │   └── new-architecture.md
│   ├── user/
│   │   ├── installation.md
│   │   ├── virtualbox.md
│   │   ├── qemu.md
│   │   └── getting-started.md
│   ├── development/
│   │   ├── coding-style.md
│   │   ├── debugging.md
│   │   ├── testing.md
│   │   └── contributing.md
│   ├── README.md
│   └── Makefile
│
├── tests/
│   ├── unit/
│   │   ├── mm/
│   │   │   ├── test_pmm.cpp
│   │   │   ├── test_vmm.cpp
│   │   │   ├── test_slab.cpp
│   │   │   └── Makefile
│   │   ├── sched/
│   │   │   ├── test_sched.cpp
│   │   │   ├── test_mutex.cpp
│   │   │   └── Makefile
│   │   ├── fs/
│   │   │   ├── test_vfs.cpp
│   │   │   ├── test_ext2.cpp
│   │   │   └── Makefile
│   │   ├── lib/
│   │   │   ├── test_string.cpp
│   │   │   ├── test_list.cpp
│   │   │   └── Makefile
│   │   └── Makefile
│   ├── integration/
│   │   ├── boot/
│   │   │   ├── test_boot.sh
│   │   │   └── test_multiboot.sh
│   │   ├── syscall/
│   │   │   ├── test_syscalls.cpp
│   │   │   └── Makefile
│   │   ├── drivers/
│   │   │   ├── test_pci.sh
│   │   │   └── Makefile
│   │   ├── fs/
│   │   │   ├── test_mount.sh
│   │   │   └── Makefile
│   │   └── Makefile
│   ├── kernel/                    # Kernel unit tests
│   │   ├── kunit/
│   │   │   ├── kunit.cpp
│   │   │   ├── kunit.hpp
│   │   │   └── Makefile
│   │   ├── test_main.cpp
│   │   └── Makefile
│   ├── scripts/
│   │   ├── run-tests.sh
│   │   ├── test-runner.py
│   │   └── coverage.sh
│   └── Makefile
│
├── scripts/
│   ├── build.sh                   # Main build script
│   ├── build-kernel.sh            # Build kernel only
│   ├── build-userland.sh          # Build userland only
│   ├── clean.sh                   # Clean build
│   ├── config.sh                  # Configure build
│   ├── menuconfig.sh              # Interactive config
│   ├── run-qemu.sh                # Run in QEMU
│   │   ├── run-qemu-bios.sh       # BIOS boot
│   │   ├── run-qemu-uefi.sh       # UEFI boot
│   │   └── run-qemu-debug.sh      # Debug mode
│   ├── run-vbox.sh                # Run in VirtualBox
│   │   ├── create-vm.sh           # Create VM
│   │   ├── start-vm.sh            # Start VM
│   │   ├── stop-vm.sh             # Stop VM
│   │   └── delete-vm.sh           # Delete VM
│   ├── run-bochs.sh               # Run in Bochs
│   ├── create-iso.sh              # ISO creation
│   ├── create-disk.sh             # Create disk image
│   ├── install-toolchain.sh       # Install cross-compiler
│   ├── check-toolchain.sh         # Verify toolchain
│   ├── debug.sh                   # Debug script
│   ├── gdb.sh                     # GDB wrapper
│   ├── symbolize.sh               # Symbolize stack traces
│   ├── checkpatch.pl              # Check coding style
│   ├── checkstack.pl              # Check stack usage
│   ├── tags.sh                    # Generate ctags
│   ├── cscope.sh                  # Generate cscope
│   └── Makefile
│
├── build/
│   ├── boot/
│   │   ├── stage1.bin
│   │   ├── stage2.bin
│   │   └── grub/
│   ├── kernel/
│   │   ├── kernel.bin             # Kernel executable (ELF)
│   │   ├── kernel.sym             # Symbol table
│   │   ├── kernel.map             # Linker map
│   │   ├── *.o                    # Object files
│   │   ├── modules/               # Kernel modules
│   │   └── Makefile
│   ├── userland/
│   │   ├── libc.a                 # C library (static)
│   │   ├── libc.so                # C library (shared)
│   │   ├── bin/                   # User binaries
│   │   ├── sbin/                  # System binaries
│   │   ├── lib/                   # Shared libraries
│   │   └── Makefile
│   └── iso/
│       └── DIT-OS.iso             # Final bootable ISO
│
├── iso/
│   ├── boot/
│   │   ├── grub/
│   │   │   └── grub.cfg
│   │   ├── kernel.bin
│   │   └── limine/
│   ├── initrd.img
│   ├── limine-bios-cd.bin
│   ├── limine-uefi-cd.bin
│   ├── EFI/
│   │   └── BOOT/
│   │       └── BOOTX64.EFI
│   └── Makefile
│
├── config/
│   ├── Kconfig                    # Top-level Kconfig
│   ├── Kconfig.debug
│   ├── Kconfig.mm
│   ├── Kconfig.sched
│   ├── Kconfig.fs
│   ├── Kconfig.net
│   ├── Kconfig.drivers
│   ├── kernel.config              # Kernel configuration
│   ├── kernel.config.default      # Default config
│   ├── kernel.config.debug        # Debug config
│   ├── kernel.config.release      # Release config
│   ├── build.config               # Build configuration
│   ├── toolchain.config           # Toolchain settings
│   ├── Makefile.common            # Common Makefile rules
│   ├── arch/
│   │   └── x86_64/
│   │       └── Kconfig
│   └── Makefile
│
├── Makefile                       # Top-level Makefile
├── CMakeLists.txt                 # Optional CMake support
├── Kbuild                         # Kernel build system
├── Kconfig                        # Kernel configuration menu
├── README.md                      # Project README
├── LICENSE                        # License file
├── CONTRIBUTING.md                # Contribution guidelines
├── CHANGELOG.md                   # Version changelog
├── AUTHORS                        # List of contributors
├── .gitignore                     # Git ignore rules
├── .gitattributes                 # Git attributes
├── .clang-format                  # Clang format config
├── .editorconfig                  # Editor configuration
└── Doxyfile                       # Doxygen documentation config

```

## Additional Data Files

### Font Files (in `kernel/arch/x86_64/graphics/fonts/`)
- Raw binary font files (.bin) for 8x8 and 8x16 fonts
- Generated from PCF/PSF/BDF sources during build

### Configuration Files (in `config/`)
- `.config` - Active kernel configuration (generated)
- `*.defconfig` - Default configurations for different profiles
- `autoconf.h` - Auto-generated header from configuration

### Build Artifacts (in `build/`)
- `.deps/` - Dependency files
- `.cmd` - Command files for incremental builds
- `*.o` - Object files
- `*.a` - Static libraries
- `*.so` - Shared libraries
- `*.bin` - Binary files
- `*.sym` - Symbol files
- `*.map` - Linker map files

## Build Output Structure

After building, the `build/` directory will contain:

```
build/
├── boot/
│   ├── stage1.bin
│   ├── stage2.bin
│   └── grub/
├── kernel/
│   ├── kernel.bin                 # Kernel executable (ELF)
│   ├── kernel.sym                 # Symbol table
│   └── *.o                        # Object files
├── userland/
│   ├── libc.a                     # C library
│   ├── bin/                       # User binaries
│   └── lib/                       # Shared libraries
└── iso/
    └── DIT-OS.iso                 # Final bootable ISO
```

## Key Components

### 1. Bootloader
- **Stage 1**: 512-byte bootloader loaded by BIOS at `0x7C00`
- **Stage 2**: Real-mode loader that switches to protected mode and loads kernel
- **GRUB**: Alternative multiboot-compliant bootloader

### 2. Kernel Architecture
- **Architecture-specific**: x86_64 assembly and C code
- **Memory Management**: Physical and virtual memory with demand paging
- **Process Scheduler**: Multitasking with priority-based scheduling
- **VFS**: Unified filesystem interface supporting multiple filesystems

### 3. Device Drivers
- **Storage**: ATA, AHCI, NVMe, USB mass storage
- **Graphics**: VGA text, VBE, framebuffer, GPU acceleration
- **Network**: Full TCP/IP stack with multiple NIC drivers
- **Input**: PS/2 keyboard/mouse, USB HID

### 4. User Space
- **libc**: Standard C library with syscalls
- **Shell**: Command interpreter with scripting
- **Utilities**: Standard Unix-like tools

### 5. Build System
- Supports both Makefile and CMake
- Cross-compiler toolchain setup scripts
- ISO creation with GRUB or custom bootloader

## VirtualBox Configuration

Create a VM with these settings:
- **Type**: Other/Unknown 64-bit
- **Memory**: 512MB minimum
- **Storage**: IDE controller with DIT-OS.iso
- **Graphics**: VMSVGA with 128MB VRAM
- **Network**: Intel PRO/1000 MT Desktop
- **Boot Order**: Optical first

## File Statistics

Estimated file counts for a complete implementation:
- **Total Files**: ~1,500-2,000 files
- **Source Files (.c, .cpp, .h)**: ~800-1,000 files
- **Assembly Files (.asm, .S)**: ~50-80 files
- **Build/Make Files**: ~100-150 files
- **Documentation**: ~50-80 files
- **Configuration**: ~30-50 files
- **Scripts**: ~40-60 files
- **Test Files**: ~100-150 files

## Development Tools Integration

### IDE Support
- **VS Code**: Settings, tasks, and launch configurations in `.vscode/`
- **CLion**: CMake integration via `CMakeLists.txt`
- **Vim/Neovim**: Ctags and cscope generation scripts

### Code Quality
- **Formatting**: `.clang-format` for consistent style
- **Linting**: Custom `checkpatch.pl` for kernel coding style
- **Static Analysis**: Support for scan-build, cppcheck

### Continuous Integration
- **GitHub Actions**: Build, test, and release workflows
- **Pre-commit Hooks**: Code style checks
- **Automated Testing**: Unit and integration test runners

## References

[1] Intel 64 and IA-32 Architectures Software Developer's Manual - https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

[2] AMD64 Architecture Programmer's Manual - https://developer.amd.com/resources/developer-guides-manuals/

[3] OSDev Wiki - https://wiki.osdev.org/Main_Page

[4] GNU GRUB Manual - https://www.gnu.org/software/grub/manual/grub/grub.html

[5] Multiboot Specification - https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

[6] VESA BIOS Extensions - https://wiki.osdev.org/VESA_Video_Extensions

[7] PCI Local Bus Specification - https://www.pcisig.com/specifications/

[8] AHCI Specification - https://www.intel.com/content/www/us/en/io/serial-ata/serial-ata-ahci-spec-rev1-3-1.html

[9] NVMe Specification - https://nvmexpress.org/specifications/

[10] USB 3.0 Specification - https://www.usb.org/documents

[11] EXT2 Filesystem - https://www.nongnu.org/ext2-doc/ext2.html

[12] RFC 791 - Internet Protocol - https://tools.ietf.org/html/rfc791

[13] RFC 793 - Transmission Control Protocol - https://tools.ietf.org/html/rfc793

[14] RFC 768 - User Datagram Protocol - https://tools.ietf.org/html/rfc768

[15] RFC 826 - Address Resolution Protocol - https://tools.ietf.org/html/rfc826

[16] ELF Specification - https://refspecs.linuxfoundation.org/elf/elf.pdf

[17] DWARF Debugging Format - https://dwarfstd.org/

[18] Limine Bootloader - https://github.com/limine-bootloader/limine

[19] LLVM/Clang Cross Compilation - https://clang.llvm.org/docs/CrossCompilation.html

[20] System V ABI (x86-64) - https://refspecs.linuxbase.org/elf/x86_64-abi-0.99.pdf

[21] GNU Binutils - https://www.gnu.org/software/binutils/

[22] GCC Cross-Compiler - https://gcc.gnu.org/install/

[23] POSIX.1-2017 Standard - https://pubs.opengroup.org/onlinepubs/9699919799/

[24] Linux Kernel Coding Style - https://www.kernel.org/doc/html/latest/process/coding-style.html

[25] UEFI Specification - https://uefi.org/specifications

[26] ACPI Specification - https://uefi.org/specifications

[27] Serial ATA Specification - https://www.sata-io.org/technology

[28] FAT32 File System Specification - https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/fatgen103.doc

[29] ISO 9660 Standard - https://wiki.osdev.org/ISO_9660

[30] VirtIO Specification - https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf
