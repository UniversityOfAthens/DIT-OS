# DIT-OS Project TODO

Comprehensive task list for building the DIT Operating System based on PROJECT_STRUCTURE.md.

**Current Progress: ~7% complete** (154 of ~1,500-2,000 estimated files)

**Boot path:** We use the **custom bootloader** (stage1 + stage2). GRUB is **not** used for booting; GRUB/boot/ entries below are optional (e.g. for ISO or alternate setups).

---

## COMPLETED (131 files)

### Project Root
- [x] `README.md`
- [x] `CHANGELOG.md`
- [x] `PROJECT_STRUCTURE.md`
- [x] `Makefile`
- [x] `CMakeLists.txt`

### Boot Configuration (optional / legacy)
- [x] `boot/grub/grub.cfg` - Basic GRUB configuration (not used for boot)
- [x] `boot/grub/themes/dit/theme.txt` - Theme configuration
- [x] `boot/grub/grubenv` - GRUB environment file

---

## TODO: BOOT SYSTEM

### Bootloader (Primary - Custom)
- [x] `bootloader/src/stage1/stage1.asm` - Stage 1 bootloader (512 bytes) _(was boot.asm)_
- [x] `bootloader/src/stage1/boot.hpp` - Boot sector header definitions
- [x] `bootloader/src/stage1/Makefile`

#### Stage 2 Bootloader
- [x] `bootloader/src/stage2/stage2.asm` - Stage 2 bootloader (real mode)
- [x] `bootloader/src/stage2/stage2.cpp` - C wrapper for stage 2
- [x] `bootloader/src/stage2/pmode.asm` - Switch to protected mode _(was protected_mode.asm)_
- [x] `bootloader/src/stage2/long_mode.asm` - Switch to long mode
- [x] `bootloader/src/stage2/a20.asm` - A20 line enable
- [x] `bootloader/src/stage2/disk.cpp` - Disk I/O operations
- [x] `bootloader/src/stage2/disk.asm` - Disk I/O assembly routines
- [x] `bootloader/src/stage2/fs.cpp` - Simple filesystem reader
- [x] `bootloader/src/stage2/fat.cpp` - FAT12/16/32 reader
- [x] `bootloader/src/stage2/ext2.cpp` - EXT2 reader
- [x] `bootloader/src/stage2/iso9660.cpp` - ISO9660 reader
- [x] `bootloader/src/stage2/memory.cpp` - Memory map detection
- [x] `bootloader/src/stage2/memory.asm` - E820 memory map
- [x] `bootloader/src/stage2/video.cpp` - Video mode setup
- [x] `bootloader/src/stage2/video.asm` - VESA/VGA video modes
- [x] `bootloader/src/stage2/console.cpp` - Boot console output
- [x] `bootloader/src/stage2/elf.cpp` - ELF loader
- [x] `bootloader/src/stage2/gdt.cpp` - Global Descriptor Table setup
- [x] `bootloader/src/stage2/gdt.asm`
- [x] `bootloader/src/stage2/paging.cpp` - Early paging setup
- [x] `bootloader/src/stage2/Makefile`

#### Bootloader Headers
- [x] `bootloader/include/boot.hpp`
- [x] `bootloader/include/disk.hpp`
- [x] `bootloader/include/fs.hpp`
- [x] `bootloader/include/memory.hpp`
- [x] `bootloader/include/video.hpp`
- [x] `bootloader/include/elf.hpp`
- [x] `bootloader/include/types.hpp`

#### Bootloader Scripts
- [x] `bootloader/scripts/build.sh`
- [x] `bootloader/scripts/sign.sh` - Boot sector signing
- [x] `bootloader/build/` - Build directory (created by Makefile)

#### Boot C++ kernel (full boot path)
- [x] `bootloader/src/loader/loader.asm` - ELF loader + multiboot launcher (runs at 0x10000)
- [x] `bootloader/src/loader/Makefile` - Build loader.bin
- [x] Bootloader top-level Makefile - Use kernel from kernel/Makefile (ELF); disk layout: stage1, stage2, loader, kernel
- [x] `bootloader/src/stage2/stage2.asm` - Load loader + kernel then jump to loader (not raw kernel)

#### Bootloader (optional in structure)
- [ ] `bootloader/src/grub/grub.cfg` - GRUB config under bootloader (optional; we use custom boot)

### Optional: GRUB / boot/ (not used - custom bootloader is primary)
- [ ] Add boot entries to `boot/grub/grub.cfg` - Only if adding GRUB for ISO
- [ ] Fix `GRUB_THEME` path - Only if using GRUB
- [ ] `boot/grub/themes/dit/assets/delis.png` - Only if using GRUB theme
- [ ] `boot/grub/themes/dit/fonts/ubuntu-regular.pf2` - Only if using GRUB
- [ ] `boot/grub/themes/dit/fonts/ubuntu-bold.pf2` - Only if using GRUB
- [ ] Complete `boot/grub/themes/dit/theme.txt` styling - Only if using GRUB

### Boot Directory
- [ ] `boot/boot.asm` - Remove or repurpose (custom boot is in bootloader/)
- [ ] `boot/Makefile` - Build boot assets if needed

### Scripts (from PROJECT_STRUCTURE)
- [x] `scripts/build.sh` - Main build script (kernel + bootloader + disk)
- [ ] `scripts/build-kernel.sh` - Build kernel only
- [ ] `scripts/build-userland.sh` - Build userland only
- [ ] `scripts/clean.sh` - Clean build
- [ ] `scripts/config.sh` - Configure build
- [ ] `scripts/menuconfig.sh` - Interactive config
- [x] `scripts/run-qemu.sh` - Run in QEMU (custom bootloader disk/floppy)
- [ ] `scripts/run-qemu-bios.sh` - QEMU BIOS boot
- [ ] `scripts/run-qemu-uefi.sh` - QEMU UEFI boot
- [ ] `scripts/run-qemu-debug.sh` - QEMU debug mode
- [ ] `scripts/run-vbox.sh` - Run in VirtualBox
- [ ] `scripts/run-vbox/create-vm.sh` - Create VM
- [ ] `scripts/run-vbox/start-vm.sh` - Start VM
- [ ] `scripts/run-vbox/stop-vm.sh` - Stop VM
- [ ] `scripts/run-vbox/delete-vm.sh` - Delete VM
- [ ] `scripts/run-bochs.sh` - Run in Bochs
- [ ] `scripts/create-iso.sh` - ISO creation
- [x] `scripts/create-disk.sh` - Create disk image (stage1, stage2, loader, kernel)
- [ ] `scripts/install-toolchain.sh` - Install cross-compiler
- [ ] `scripts/check-toolchain.sh` - Verify toolchain
- [ ] `scripts/debug.sh` - Debug script
- [ ] `scripts/gdb.sh` - GDB wrapper
- [ ] `scripts/symbolize.sh` - Symbolize stack traces
- [ ] `scripts/checkpatch.pl` - Check coding style
- [ ] `scripts/checkstack.pl` - Check stack usage
- [ ] `scripts/tags.sh` - Generate ctags
- [ ] `scripts/cscope.sh` - Generate cscope
- [ ] `scripts/Makefile`

### Tools (from PROJECT_STRUCTURE)
- [ ] `tools/cross-compiler/build-binutils.sh`
- [ ] `tools/cross-compiler/build-gcc.sh`
- [ ] `tools/cross-compiler/build-gdb.sh`
- [ ] `tools/cross-compiler/download.sh`
- [ ] `tools/cross-compiler/config.sh`
- [ ] `tools/cross-compiler/README.md`
- [ ] `tools/build-iso/mkiso.sh`
- [ ] `tools/build-iso/mkiso.py`
- [ ] `tools/build-iso/isohybrid.cpp`
- [ ] `tools/build-iso/Makefile`
- [ ] `tools/mkinitrd/mkinitrd.cpp`
- [ ] `tools/mkinitrd/mkinitrd.sh`
- [ ] `tools/mkinitrd/cpio.cpp`
- [ ] `tools/mkinitrd/Makefile`
- [ ] `tools/font-converter/fontconv.cpp`
- [ ] `tools/font-converter/psf2c.cpp`
- [ ] `tools/font-converter/bdf2c.cpp`
- [ ] `tools/font-converter/Makefile`
- [ ] `tools/debug/gdb-init`
- [ ] `tools/debug/bochsrc`
- [ ] `tools/debug/bochs-debugger.sh`
- [ ] `tools/debug/qemu-gdb.sh`
- [ ] `tools/debug/trace.py`
- [ ] `tools/debug/README.md`
- [ ] `tools/kconfig/mconf.cpp`
- [ ] `tools/kconfig/conf.cpp`
- [ ] `tools/kconfig/Makefile`
- [ ] `tools/Makefile`

### Build / ISO layout (custom bootloader)
- [ ] `build/boot/stage1.bin` - Produced by bootloader Makefile
- [ ] `build/boot/stage2.bin` - Produced by bootloader Makefile
- [ ] `build/iso/` - ISO output (when using create-iso with custom boot)
- [ ] `iso/` - ISO source layout (kernel.bin, initrd.img, etc.) for custom or hybrid boot

---

## TODO: KERNEL (Core OS)

### Architecture: x86_64

#### Boot Components
- [x] `kernel/arch/x86_64/boot/multiboot.asm` - Multiboot header
- [x] `kernel/arch/x86_64/boot/multiboot2.asm` - Multiboot2 header
- [x] `kernel/arch/x86_64/boot/entry.asm` - Kernel entry point
- [x] `kernel/arch/x86_64/boot/init.cpp` - Early initialization
- [x] `kernel/arch/x86_64/boot/main.cpp` - Kernel main
- [x] `kernel/arch/x86_64/boot/trampoline.asm` - SMP trampoline
- [x] `kernel/arch/x86_64/boot/linker.ld` - Linker script
- [x] `kernel/arch/x86_64/boot/Makefile`

#### CPU Management
- [x] `kernel/arch/x86_64/cpu/gdt.asm` - Global Descriptor Table
- [x] `kernel/arch/x86_64/cpu/gdt.cpp`
- [x] `kernel/arch/x86_64/cpu/idt.asm` - Interrupt Descriptor Table
- [x] `kernel/arch/x86_64/cpu/idt.cpp`
- [x] `kernel/arch/x86_64/cpu/tss.cpp` - Task State Segment
- [x] `kernel/arch/x86_64/cpu/tss.asm`
- [x] `kernel/arch/x86_64/cpu/cpuid.cpp` - CPU feature detection
- [x] `kernel/arch/x86_64/cpu/msr.cpp` - Model Specific Registers
- [x] `kernel/arch/x86_64/cpu/msr.asm`
- [x] `kernel/arch/x86_64/cpu/pic.cpp` - Programmable Interrupt Controller
- [x] `kernel/arch/x86_64/cpu/pic.asm`
- [x] `kernel/arch/x86_64/cpu/apic.cpp` - Advanced PIC (stub)
- [x] `kernel/arch/x86_64/cpu/ioapic.cpp` - I/O APIC (stub)
- [x] `kernel/arch/x86_64/cpu/smp.cpp` - Symmetric Multi-Processing (stub)
- [x] `kernel/arch/x86_64/cpu/smp.asm`
- [x] `kernel/arch/x86_64/cpu/lapic.cpp` - Local APIC (stub)
- [x] `kernel/arch/x86_64/cpu/cpuid.asm`
- [x] `kernel/arch/x86_64/cpu/control.cpp` - Control registers
- [x] `kernel/arch/x86_64/cpu/control.asm`
- [x] `kernel/arch/x86_64/cpu/mtrr.cpp` - Memory Type Range Registers (stub)
- [x] `kernel/arch/x86_64/cpu/pat.cpp` - Page Attribute Table (stub)
- [x] `kernel/arch/x86_64/cpu/cpufeatures.cpp`

#### Memory Management (MM)
- [x] `kernel/arch/x86_64/mm/paging.asm` - Page table management
- [x] `kernel/arch/x86_64/mm/paging.cpp`
- [x] `kernel/arch/x86_64/mm/pmm.cpp` - Physical Memory Manager
- [x] `kernel/arch/x86_64/mm/vmm.cpp` - Virtual Memory Manager
- [x] `kernel/arch/x86_64/mm/heap.cpp` - Kernel heap allocator
- [x] `kernel/arch/x86_64/mm/dma.cpp` - Direct Memory Access
- [x] `kernel/arch/x86_64/mm/mtrr.asm`
- [x] `kernel/arch/x86_64/mm/tlb.asm` - TLB management

#### Device Drivers (Dev)
##### Serial/Input
- [x] `kernel/arch/x86_64/dev/serial.cpp` - Serial port (COM1/COM2)
- [x] `kernel/arch/x86_64/dev/serial.hpp`
- [x] `kernel/arch/x86_64/dev/keyboard.cpp` - PS/2 keyboard driver
- [x] `kernel/arch/x86_64/dev/keyboard.hpp`
- [x] `kernel/arch/x86_64/dev/mouse.cpp` - PS/2 mouse driver
- [x] `kernel/arch/x86_64/dev/mouse.hpp`
- [x] `kernel/arch/x86_64/dev/timer.cpp` - PIT timer
- [x] `kernel/arch/x86_64/dev/timer.hpp`
- [x] `kernel/arch/x86_64/dev/rtc.cpp` - Real-Time Clock
- [x] `kernel/arch/x86_64/dev/rtc.hpp`

##### Storage/Bus
- [x] `kernel/arch/x86_64/dev/pci.cpp` - PCI bus enumeration
- [x] `kernel/arch/x86_64/dev/pci_ids.hpp` - PCI device IDs
- [x] `kernel/arch/x86_64/dev/ata.cpp` - ATA/IDE controller
- [x] `kernel/arch/x86_64/dev/ata.hpp`
- [x] `kernel/arch/x86_64/dev/ahci.cpp` - AHCI/SATA controller
- [x] `kernel/arch/x86_64/dev/ahci.hpp`
- [x] `kernel/arch/x86_64/dev/nvme.cpp` - NVMe controller
- [x] `kernel/arch/x86_64/dev/nvme.hpp`
- [x] `kernel/arch/x86_64/dev/floppy.cpp` - Floppy disk driver

##### USB Subsystem
- [ ] `kernel/arch/x86_64/dev/usb/uhci.cpp` - USB UHCI controller
- [ ] `kernel/arch/x86_64/dev/usb/ohci.cpp` - USB OHCI controller
- [ ] `kernel/arch/x86_64/dev/usb/ehci.cpp` - USB EHCI controller
- [ ] `kernel/arch/x86_64/dev/usb/xhci.cpp` - USB XHCI controller
- [ ] `kernel/arch/x86_64/dev/usb/hub.cpp` - USB hub driver
- [ ] `kernel/arch/x86_64/dev/usb/hid.cpp` - USB HID driver
- [ ] `kernel/arch/x86_64/dev/usb/mass_storage.cpp` - USB mass storage

##### Network
- [ ] `kernel/arch/x86_64/dev/network/e1000.cpp` - Intel E1000 NIC
- [ ] `kernel/arch/x86_64/dev/network/rtl8139.cpp` - Realtek RTL8139
- [ ] `kernel/arch/x86_64/dev/network/virtio_net.cpp` - VirtIO network

#### Graphics
- [ ] `kernel/arch/x86_64/graphics/vga.cpp` - VGA text mode
- [ ] `kernel/arch/x86_64/graphics/vga.hpp`
- [ ] `kernel/arch/x86_64/graphics/vbe.cpp` - VESA BIOS Extensions
- [ ] `kernel/arch/x86_64/graphics/vbe.hpp`
- [ ] `kernel/arch/x86_64/graphics/framebuffer.cpp` - Linear framebuffer
- [ ] `kernel/arch/x86_64/graphics/bochs_vbe.cpp` - Bochs VBE extensions

##### Fonts
- [ ] `kernel/arch/x86_64/graphics/fonts/font_8x16.cpp`
- [ ] `kernel/arch/x86_64/graphics/fonts/font_8x16.bin` - Raw font data
- [ ] `kernel/arch/x86_64/graphics/fonts/font_8x8.cpp`
- [ ] `kernel/arch/x86_64/graphics/fonts/font_8x8.bin`
- [ ] `kernel/arch/x86_64/graphics/fonts/fonts.hpp`

##### GPU Support
- [ ] `kernel/arch/x86_64/graphics/gpu/bochs_gpu.cpp`
- [ ] `kernel/arch/x86_64/graphics/gpu/vmware_svga.cpp`
- [ ] `kernel/arch/x86_64/graphics/gpu/virtio_gpu.cpp`

#### Debug Support
- [ ] `kernel/arch/x86_64/debug/debug.cpp`
- [ ] `kernel/arch/x86_64/debug/gdbstub.cpp` - GDB remote stub
- [ ] `kernel/arch/x86_64/debug/backtrace.cpp` - Stack unwinding
- [ ] `kernel/arch/x86_64/debug/Makefile`
- [ ] `kernel/arch/x86_64/Makefile`

### Core Kernel Components

- [x] `kernel/core/kernel.cpp` - Kernel main entry
- [x] `kernel/core/panic.cpp` - Kernel panic handler
- [x] `kernel/core/panic.hpp`
- [x] `kernel/core/printk.cpp` - Kernel printf implementation
- [x] `kernel/core/printk.hpp`
- [x] `kernel/core/console.cpp` - Kernel console system
- [x] `kernel/core/console.hpp`
- [x] `kernel/core/kassert.cpp` - Kernel assertions
- [x] `kernel/core/kassert.hpp`
- [x] `kernel/core/cmdline.cpp` - Kernel command line parsing
- [x] `kernel/core/cmdline.hpp`
- [x] `kernel/core/version.cpp` - Version information
- [x] `kernel/core/version.hpp`
- [x] `kernel/core/logo.cpp` - Kernel boot logo
- [x] `kernel/core/logo.hpp`
- [x] `kernel/core/config.cpp` - Kernel configuration
- [x] `kernel/core/config.hpp`
- [x] `kernel/core/modules.cpp` - Kernel module support
- [x] `kernel/core/modules.hpp`
- [x] `kernel/core/symbols.cpp` - Kernel symbol table
- [x] `kernel/core/symbols.hpp`
- [x] `kernel/core/compiler.hpp` - Compiler attributes
- [x] `kernel/core/compiler_gcc.hpp`
- [x] `kernel/core/compiler_clang.hpp`
- [x] `kernel/core/Makefile`

### Include Headers

- [x] `kernel/include/kernel.hpp`
- [x] `kernel/include/types.hpp`
- [x] `kernel/include/stddef.hpp`
- [x] `kernel/include/stdint.hpp`
- [x] `kernel/include/stdbool.hpp`
- [x] `kernel/include/string.hpp`
- [x] `kernel/include/stdlib.hpp`
- [x] `kernel/include/errno.hpp`
- [x] `kernel/include/errno_defs.hpp` - Error code definitions
- [x] `kernel/include/limits.hpp`
- [x] `kernel/include/ctype.hpp`
- [x] `kernel/include/math.hpp`
- [x] `kernel/include/assert.hpp`
- [x] `kernel/include/kconfig.hpp` - Kernel configuration header
- [x] `kernel/include/kconfig_defaults.hpp` - Default config values
- [x] `kernel/include/config.hpp` - Auto-generated config

#### Architecture Headers
- [x] `kernel/include/arch/x86_64/cpu.hpp`
- [x] `kernel/include/arch/x86_64/gdt.hpp`
- [x] `kernel/include/arch/x86_64/idt.hpp`
- [x] `kernel/include/arch/x86_64/tss.hpp`
- [x] `kernel/include/arch/x86_64/io.hpp`
- [x] `kernel/include/arch/x86_64/paging.hpp`
- [x] `kernel/include/arch/x86_64/msr.hpp`
- [x] `kernel/include/arch/x86_64/cpuid.hpp`
- [x] `kernel/include/arch/x86_64/control.hpp`
- [x] `kernel/include/arch/x86_64/apic.hpp`
- [x] `kernel/include/arch/x86_64/pic.hpp`
- [x] `kernel/include/arch/x86_64/smp.hpp`
- [x] `kernel/include/arch/x86_64/lapic.hpp`
- [x] `kernel/include/arch/x86_64/ioapic.hpp`
- [x] `kernel/include/arch/x86_64/arch.hpp`

#### Memory Management Headers
- [x] `kernel/include/mm/pmm.hpp`
- [x] `kernel/include/mm/vmm.hpp`
- [x] `kernel/include/mm/kmalloc.hpp`
- [x] `kernel/include/mm/slab.hpp`
- [x] `kernel/include/mm/page.hpp`
- [x] `kernel/include/mm/page-flags.hpp`
- [x] `kernel/include/mm/page-defs.hpp`
- [x] `kernel/include/mm/memblock.hpp`
- [x] `kernel/include/mm/early_mm.hpp`
- [x] `kernel/include/mm/mmu.hpp`

#### Device Headers
- [ ] `kernel/include/dev/device.hpp`
- [ ] `kernel/include/dev/driver.hpp`
- [ ] `kernel/include/dev/bus.hpp`
- [ ] `kernel/include/dev/class.hpp`
- [x] `kernel/include/dev/serial.hpp`
- [x] `kernel/include/dev/keyboard.hpp`
- [ ] `kernel/include/dev/keycodes.hpp`
- [x] `kernel/include/dev/timer.hpp`
- [x] `kernel/include/dev/rtc.hpp`
- [x] `kernel/include/dev/pci.hpp`
- [x] `kernel/include/dev/pci_ids.hpp`
- [x] `kernel/include/dev/ata.hpp`
- [ ] `kernel/include/dev/usb.hpp`
- [ ] `kernel/include/dev/usb_spec.hpp`
- [ ] `kernel/include/dev/blockdev.hpp`
- [ ] `kernel/include/dev/chardev.hpp`

#### Filesystem Headers
- [ ] `kernel/include/fs/vfs.hpp`
- [ ] `kernel/include/fs/inode.hpp`
- [ ] `kernel/include/fs/dentry.hpp`
- [ ] `kernel/include/fs/superblock.hpp`
- [ ] `kernel/include/fs/file.hpp`
- [ ] `kernel/include/fs/mount.hpp`
- [ ] `kernel/include/fs/fs.hpp`
- [ ] `kernel/include/fs/namei.hpp`
- [ ] `kernel/include/fs/stat.hpp`
- [ ] `kernel/include/fs/fcntl.hpp`
- [ ] `kernel/include/fs/ioctl.hpp`
- [ ] `kernel/include/fs/dirent.hpp`
- [ ] `kernel/include/fs/poll.hpp`
- [ ] `kernel/include/fs/select.hpp`
- [ ] `kernel/include/fs/buffer.hpp`

#### Graphics Headers
- [ ] `kernel/include/graphics/vga.hpp`
- [ ] `kernel/include/graphics/framebuffer.hpp`
- [ ] `kernel/include/graphics/font.hpp`
- [ ] `kernel/include/graphics/color.hpp`
- [ ] `kernel/include/graphics/drawing.hpp`
- [ ] `kernel/include/graphics/cursor.hpp`
- [ ] `kernel/include/graphics/terminal.hpp`
- [ ] `kernel/include/graphics/terminal-buffer.hpp`

#### Network Headers
- [ ] `kernel/include/net/socket.hpp`
- [ ] `kernel/include/net/packet.hpp`
- [ ] `kernel/include/net/ip.hpp`
- [ ] `kernel/include/net/tcp.hpp`
- [ ] `kernel/include/net/udp.hpp`
- [ ] `kernel/include/net/icmp.hpp`
- [ ] `kernel/include/net/arp.hpp`
- [ ] `kernel/include/net/ethernet.hpp`
- [ ] `kernel/include/net/checksum.hpp`
- [ ] `kernel/include/net/net.hpp`
- [ ] `kernel/include/net/netdevice.hpp`
- [ ] `kernel/include/net/skbuff.hpp`
- [ ] `kernel/include/net/sock.hpp`
- [ ] `kernel/include/net/proto.hpp`
- [ ] `kernel/include/net/route.hpp`
- [ ] `kernel/include/net/fib.hpp`
- [ ] `kernel/include/net/if_ether.hpp`
- [ ] `kernel/include/net/if.hpp`
- [ ] `kernel/include/net/in.hpp`
- [ ] `kernel/include/net/inet.hpp`
- [ ] `kernel/include/net/netfilter.hpp`

#### Syscall Headers
- [ ] `kernel/include/syscall/syscall.hpp`
- [ ] `kernel/include/syscall/syscall_table.hpp`
- [ ] `kernel/include/syscall/syscall_numbers.hpp`
- [ ] `kernel/include/syscall/syscall_defs.hpp`
- [ ] `kernel/include/syscall/syscall_trace.hpp`

#### Scheduler Headers
- [ ] `kernel/include/sched/sched.hpp`
- [ ] `kernel/include/sched/task.hpp`
- [ ] `kernel/include/sched/process.hpp`
- [ ] `kernel/include/sched/thread.hpp`
- [ ] `kernel/include/sched/signal.hpp`
- [ ] `kernel/include/sched/wait.hpp`
- [ ] `kernel/include/sched/timer.hpp`
- [ ] `kernel/include/sched/cpumask.hpp`

#### Library Headers
- [ ] `kernel/include/lib/list.hpp`
- [ ] `kernel/include/lib/queue.hpp`
- [ ] `kernel/include/lib/stack.hpp`
- [ ] `kernel/include/lib/hash.hpp`
- [ ] `kernel/include/lib/bitmap.hpp`
- [ ] `kernel/include/lib/string.hpp`
- [ ] `kernel/include/lib/spinlock.hpp`
- [ ] `kernel/include/lib/mutex.hpp`
- [ ] `kernel/include/lib/semaphore.hpp`
- [ ] `kernel/include/lib/rwlock.hpp`
- [ ] `kernel/include/lib/atomic.hpp`
- [ ] `kernel/include/lib/refcount.hpp`
- [ ] `kernel/include/lib/kref.hpp`
- [ ] `kernel/include/lib/kobject.hpp`
- [ ] `kernel/include/lib/kset.hpp`
- [ ] `kernel/include/lib/klist.hpp`
- [ ] `kernel/include/lib/hlist.hpp`
- [ ] `kernel/include/lib/rbtree.hpp`
- [ ] `kernel/include/lib/bsearch.hpp`
- [ ] `kernel/include/lib/sort.hpp`
- [ ] `kernel/include/lib/ctype.hpp`
- [ ] `kernel/include/lib/div64.hpp`
- [ ] `kernel/include/lib/crc32.hpp`
- [ ] `kernel/include/lib/crc16.hpp`
- [ ] `kernel/include/lib/checksum.hpp`

### Memory Management (MM)
- [ ] `kernel/mm/pmm.cpp` - Physical memory management
- [ ] `kernel/mm/vmm.cpp` - Virtual memory management
- [ ] `kernel/mm/kmalloc.cpp` - Kernel allocator (slab/buddy)
- [ ] `kernel/mm/slab.cpp` - Slab allocator
- [ ] `kernel/mm/buddy.cpp` - Buddy allocator
- [ ] `kernel/mm/page_fault.cpp` - Page fault handler
- [ ] `kernel/mm/swap.cpp` - Swap space management
- [ ] `kernel/mm/memblock.cpp` - Early memory allocator
- [ ] `kernel/mm/early_mm.cpp` - Early memory setup
- [ ] `kernel/mm/page_alloc.cpp` - Page allocation
- [ ] `kernel/mm/page-writeback.cpp` - Page writeback
- [ ] `kernel/mm/mmap.cpp` - Memory mapping
- [ ] `kernel/mm/mremap.cpp` - Remap pages
- [ ] `kernel/mm/mlock.cpp` - Lock pages
- [ ] `kernel/mm/madvise.cpp` - Memory advice
- [ ] `kernel/mm/mincore.cpp` - Check page residency
- [ ] `kernel/mm/memory.cpp` - Memory info
- [ ] `kernel/mm/percpu.cpp` - Per-CPU allocations
- [ ] `kernel/mm/vmalloc.cpp` - vmalloc allocator
- [ ] `kernel/mm/ioremap.cpp` - I/O remapping
- [ ] `kernel/mm/highmem.cpp` - High memory handling
- [ ] `kernel/mm/page_io.cpp` - Page I/O
- [ ] `kernel/mm/truncate.cpp` - Truncate pages
- [ ] `kernel/mm/filemap.cpp` - File-backed mappings
- [ ] `kernel/mm/readahead.cpp` - Read-ahead
- [ ] `kernel/mm/swapfile.cpp` - Swap file support
- [ ] `kernel/mm/swap_state.cpp` - Swap state
- [ ] `kernel/mm/swap_slots.cpp` - Swap slot management
- [ ] `kernel/mm/page.cpp` - Page operations
- [ ] `kernel/mm/page_alloc_ext.cpp` - Extended allocators
- [ ] `kernel/mm/memory_hotplug.cpp` - Hot-plug support
- [ ] `kernel/mm/Makefile`

### Scheduler (Sched)
- [ ] `kernel/sched/sched.cpp` - Main scheduler
- [ ] `kernel/sched/task.cpp` - Task/thread management
- [ ] `kernel/sched/fork.cpp` - Process creation
- [ ] `kernel/sched/exec.cpp` - Program execution
- [ ] `kernel/sched/exit.cpp` - Process termination
- [ ] `kernel/sched/wait.cpp` - Wait for child processes
- [ ] `kernel/sched/signal.cpp` - Signal handling
- [ ] `kernel/sched/signal.hpp`
- [ ] `kernel/sched/timer.cpp` - Timer/scheduling
- [ ] `kernel/sched/context.asm` - Context switching
- [ ] `kernel/sched/swtch.asm` - Context switch
- [ ] `kernel/sched/entry.asm` - Task entry points
- [ ] `kernel/sched/mutex.cpp` - Mutex implementation
- [ ] `kernel/sched/semaphore.cpp` - Semaphore implementation
- [ ] `kernel/sched/rwlock.cpp` - Read-write locks
- [ ] `kernel/sched/condvar.cpp` - Condition variables
- [ ] `kernel/sched/futex.cpp` - Fast userspace mutex
- [ ] `kernel/sched/process.cpp` - Process management
- [ ] `kernel/sched/thread.cpp` - Thread management
- [ ] `kernel/sched/pid.cpp` - PID allocation
- [ ] `kernel/sched/loadavg.cpp` - Load average calculation
- [ ] `kernel/sched/clock.cpp` - Scheduling clock
- [ ] `kernel/sched/deadline.cpp` - Deadline scheduler
- [ ] `kernel/sched/fair.cpp` - CFS scheduler
- [ ] `kernel/sched/rt.cpp` - Real-time scheduler
- [ ] `kernel/sched/idle.cpp` - Idle task
- [ ] `kernel/sched/stop_task.cpp` - Stop task
- [ ] `kernel/sched/cgroup.cpp` - Control groups
- [ ] `kernel/sched/namespace.cpp` - Namespaces
- [ ] `kernel/sched/core.cpp` - Core scheduler
- [ ] `kernel/sched/autogroup.cpp` - Auto-grouping
- [ ] `kernel/sched/waitq.cpp` - Wait queue implementation
- [ ] `kernel/sched/completion.cpp` - Completion mechanism
- [ ] `kernel/sched/smp.cpp` - SMP scheduling
- [ ] `kernel/sched/cpuidle.cpp` - CPU idle management
- [ ] `kernel/sched/cpufreq.cpp` - CPU frequency scaling
- [ ] `kernel/sched/sched.hpp` - Internal scheduler headers
- [ ] `kernel/sched/sched_api.hpp`
- [ ] `kernel/sched/Makefile`

### Filesystem (FS)
- [ ] `kernel/fs/vfs.cpp` - Virtual File System
- [ ] `kernel/fs/file.cpp` - File operations
- [ ] `kernel/fs/inode.cpp` - Inode management
- [ ] `kernel/fs/dentry.cpp` - Directory entry cache
- [ ] `kernel/fs/superblock.cpp` - Filesystem superblocks
- [ ] `kernel/fs/mount.cpp` - Mount management
- [ ] `kernel/fs/path.cpp` - Path resolution
- [ ] `kernel/fs/buffer.cpp` - Buffer cache
- [ ] `kernel/fs/namei.cpp` - Name resolution
- [ ] `kernel/fs/open.cpp` - Open file handling
- [ ] `kernel/fs/read_write.cpp` - Read/write operations
- [ ] `kernel/fs/readdir.cpp` - Directory reading
- [ ] `kernel/fs/stat.cpp` - File status
- [ ] `kernel/fs/ioctl.cpp` - I/O control
- [ ] `kernel/fs/fcntl.cpp` - File control
- [ ] `kernel/fs/pipe.cpp` - Pipe operations
- [ ] `kernel/fs/fifo.cpp` - Named pipes
- [ ] `kernel/fs/splice.cpp` - Splice operation
- [ ] `kernel/fs/sync.cpp` - File synchronization
- [ ] `kernel/fs/fs-writeback.cpp` - Writeback management
- [ ] `kernel/fs/fsync.cpp` - fsync operations
- [ ] `kernel/fs/lock.cpp` - File locking
- [ ] `kernel/fs/dcache.cpp` - Directory cache
- [ ] `kernel/fs/namespace.cpp` - Mount namespaces
- [ ] `kernel/fs/seq_file.cpp` - Sequential file I/O
- [ ] `kernel/fs/libfs.cpp` - FS library functions
- [ ] `kernel/fs/file_table.cpp` - File descriptor table
- [ ] `kernel/fs/mpage.cpp` - Multi-page I/O
- [ ] `kernel/fs/direct-io.cpp` - Direct I/O
- [ ] `kernel/fs/bad_inode.cpp` - Bad inode handling
- [ ] `kernel/fs/Makefile`

#### devfs (Device Filesystem)
- [ ] `kernel/fs/devfs/devfs.cpp`
- [ ] `kernel/fs/devfs/devfs.hpp`
- [ ] `kernel/fs/devfs/Makefile`

#### ramfs (RAM Filesystem)
- [ ] `kernel/fs/ramfs/ramfs.cpp`
- [ ] `kernel/fs/ramfs/ramfs.hpp`
- [ ] `kernel/fs/ramfs/inode.cpp`
- [ ] `kernel/fs/ramfs/Makefile`

#### ext2 (EXT2 Filesystem)
- [ ] `kernel/fs/ext2/ext2.cpp`
- [ ] `kernel/fs/ext2/ext2.hpp`
- [ ] `kernel/fs/ext2/super.cpp`
- [ ] `kernel/fs/ext2/inode.cpp`
- [ ] `kernel/fs/ext2/file.cpp`
- [ ] `kernel/fs/ext2/dir.cpp`
- [ ] `kernel/fs/ext2/namei.cpp`
- [ ] `kernel/fs/ext2/balloc.cpp` - Block allocation
- [ ] `kernel/fs/ext2/ialloc.cpp` - Inode allocation
- [ ] `kernel/fs/ext2/truncate.cpp`
- [ ] `kernel/fs/ext2/symlink.cpp`
- [ ] `kernel/fs/ext2/ioctl.cpp`
- [ ] `kernel/fs/ext2/Makefile`

#### fat32 (FAT32 Filesystem)
- [ ] `kernel/fs/fat32/fat32.cpp`
- [ ] `kernel/fs/fat32/fat32.hpp`
- [ ] `kernel/fs/fat32/cache.cpp`
- [ ] `kernel/fs/fat32/dir.cpp`
- [ ] `kernel/fs/fat32/file.cpp`
- [ ] `kernel/fs/fat32/inode.cpp`
- [ ] `kernel/fs/fat32/misc.cpp`
- [ ] `kernel/fs/fat32/namei.cpp`
- [ ] `kernel/fs/fat32/Makefile`

#### iso9660 (CD-ROM)
- [ ] `kernel/fs/iso9660/iso9660.cpp`
- [ ] `kernel/fs/iso9660/iso9660.hpp`
- [ ] `kernel/fs/iso9660/name.cpp`
- [ ] `kernel/fs/iso9660/inode.cpp`
- [ ] `kernel/fs/iso9660/dir.cpp`
- [ ] `kernel/fs/iso9660/Makefile`

#### tmpfs (Temporary Filesystem)
- [ ] `kernel/fs/tmpfs/tmpfs.cpp`
- [ ] `kernel/fs/tmpfs/tmpfs.hpp`
- [ ] `kernel/fs/tmpfs/Makefile`

#### procfs (Proc Filesystem)
- [ ] `kernel/fs/procfs/procfs.cpp`
- [ ] `kernel/fs/procfs/procfs.hpp`
- [ ] `kernel/fs/procfs/root.cpp`
- [ ] `kernel/fs/procfs/meminfo.cpp`
- [ ] `kernel/fs/procfs/cpuinfo.cpp`
- [ ] `kernel/fs/procfs/version.cpp`
- [ ] `kernel/fs/procfs/uptime.cpp`
- [ ] `kernel/fs/procfs/stat.cpp`
- [ ] `kernel/fs/procfs/modules.cpp`
- [ ] `kernel/fs/procfs/mounts.cpp`
- [ ] `kernel/fs/procfs/Makefile`

### Drivers

#### Character Drivers
- [ ] `kernel/drivers/char/serial.cpp`
- [ ] `kernel/drivers/char/serial.hpp`
- [ ] `kernel/drivers/char/keyboard.cpp`
- [ ] `kernel/drivers/char/keyboard.hpp`
- [ ] `kernel/drivers/char/tty.cpp` - Terminal driver
- [ ] `kernel/drivers/char/tty.hpp`
- [ ] `kernel/drivers/char/random.cpp` - Random number generator
- [ ] `kernel/drivers/char/random.hpp`
- [ ] `kernel/drivers/char/urandom.cpp` - /dev/urandom
- [ ] `kernel/drivers/char/mem.cpp` - /dev/mem, /dev/kmem
- [ ] `kernel/drivers/char/null.cpp` - /dev/null
- [ ] `kernel/drivers/char/zero.cpp` - /dev/zero
- [ ] `kernel/drivers/char/full.cpp` - /dev/full
- [ ] `kernel/drivers/char/port.cpp` - /dev/port
- [ ] `kernel/drivers/char/Makefile`

#### Block Drivers
- [ ] `kernel/drivers/block/block.cpp` - Block layer
- [ ] `kernel/drivers/block/block.hpp`
- [ ] `kernel/drivers/block/queue.cpp` - I/O request queue
- [ ] `kernel/drivers/block/scheduler.cpp` - I/O scheduler
- [ ] `kernel/drivers/block/elevator.cpp` - I/O elevator
- [ ] `kernel/drivers/block/deadline-iosched.cpp` - Deadline I/O scheduler
- [ ] `kernel/drivers/block/cfq-iosched.cpp` - CFQ I/O scheduler
- [ ] `kernel/drivers/block/noop-iosched.cpp` - No-op I/O scheduler
- [ ] `kernel/drivers/block/ramdisk.cpp` - RAM disk driver
- [ ] `kernel/drivers/block/loop.cpp` - Loopback device
- [ ] `kernel/drivers/block/genhd.cpp` - Generic hard disk
- [ ] `kernel/drivers/block/Makefile`

#### Bus Drivers
- [ ] `kernel/drivers/bus/pci.cpp` - PCI bus driver
- [ ] `kernel/drivers/bus/pci.hpp`
- [ ] `kernel/drivers/bus/pci_ids.cpp` - PCI device database
- [ ] `kernel/drivers/bus/pci_ids.hpp`
- [ ] `kernel/drivers/bus/pci_sysfs.cpp` - PCI sysfs
- [ ] `kernel/drivers/bus/pci_probe.cpp` - PCI probing
- [ ] `kernel/drivers/bus/usb.cpp` - USB core driver
- [ ] `kernel/drivers/bus/usb.hpp`
- [ ] `kernel/drivers/bus/usb_hub.cpp`
- [ ] `kernel/drivers/bus/usb_device.cpp`
- [ ] `kernel/drivers/bus/usb_hcd.cpp` - Host controller driver
- [ ] `kernel/drivers/bus/Makefile`

#### Storage Drivers
- [ ] `kernel/drivers/storage/ata.cpp`
- [ ] `kernel/drivers/storage/ata.hpp`
- [ ] `kernel/drivers/storage/libata.cpp` - ATA library
- [ ] `kernel/drivers/storage/ahci.cpp`
- [ ] `kernel/drivers/storage/ahci.hpp`
- [ ] `kernel/drivers/storage/nvme.cpp`
- [ ] `kernel/drivers/storage/nvme.hpp`
- [ ] `kernel/drivers/storage/scsi.cpp` - SCSI support
- [ ] `kernel/drivers/storage/scsi.hpp`
- [ ] `kernel/drivers/storage/scsi_scan.cpp`
- [ ] `kernel/drivers/storage/scsi_lib.cpp`
- [ ] `kernel/drivers/storage/scsi_sysfs.cpp`
- [ ] `kernel/drivers/storage/Makefile`

#### Input Drivers
- [ ] `kernel/drivers/input/keyboard.cpp`
- [ ] `kernel/drivers/input/mouse.cpp`
- [ ] `kernel/drivers/input/input.cpp` - Input core
- [ ] `kernel/drivers/input/input.hpp`
- [ ] `kernel/drivers/input/event.cpp` - Input event system
- [ ] `kernel/drivers/input/evdev.cpp` - Event device
- [ ] `kernel/drivers/input/serio.cpp` - Serial I/O
- [ ] `kernel/drivers/input/Makefile`

#### Graphics Drivers
- [ ] `kernel/drivers/graphics/vga.cpp`
- [ ] `kernel/drivers/graphics/vga.hpp`
- [ ] `kernel/drivers/graphics/vbe.cpp`
- [ ] `kernel/drivers/graphics/vbe.hpp`
- [ ] `kernel/drivers/graphics/fb.cpp` - Framebuffer
- [ ] `kernel/drivers/graphics/fb.hpp`
- [ ] `kernel/drivers/graphics/fbmem.cpp` - Framebuffer memory
- [ ] `kernel/drivers/graphics/fbcmap.cpp` - Framebuffer colormap
- [ ] `kernel/drivers/graphics/fbsysfs.cpp` - Framebuffer sysfs

##### DRM (Direct Rendering Manager)
- [ ] `kernel/drivers/graphics/drm/drm.cpp`
- [ ] `kernel/drivers/graphics/drm/drm.hpp`
- [ ] `kernel/drivers/graphics/drm/Makefile`
- [ ] `kernel/drivers/graphics/Makefile`

#### Network Drivers
- [ ] `kernel/drivers/net/network.cpp` - Network stack core
- [ ] `kernel/drivers/net/network.hpp`
- [ ] `kernel/drivers/net/arp.cpp`
- [ ] `kernel/drivers/net/ip.cpp`
- [ ] `kernel/drivers/net/ip_input.cpp`
- [ ] `kernel/drivers/net/ip_output.cpp`
- [ ] `kernel/drivers/net/ip_fragment.cpp`
- [ ] `kernel/drivers/net/ip_options.cpp`
- [ ] `kernel/drivers/net/icmp.cpp`
- [ ] `kernel/drivers/net/tcp.cpp`
- [ ] `kernel/drivers/net/tcp_input.cpp`
- [ ] `kernel/drivers/net/tcp_output.cpp`
- [ ] `kernel/drivers/net/tcp_timer.cpp`
- [ ] `kernel/drivers/net/tcp_cong.cpp` - TCP congestion control
- [ ] `kernel/drivers/net/udp.cpp`
- [ ] `kernel/drivers/net/socket.cpp`
- [ ] `kernel/drivers/net/socket.hpp`
- [ ] `kernel/drivers/net/af_inet.cpp` - INET address family
- [ ] `kernel/drivers/net/netdev.cpp` - Network devices
- [ ] `kernel/drivers/net/netdev.hpp`
- [ ] `kernel/drivers/net/packet.cpp` - Packet buffer
- [ ] `kernel/drivers/net/route.cpp` - Routing table
- [ ] `kernel/drivers/net/fib.cpp` - Forwarding Information Base
- [ ] `kernel/drivers/net/checksum.cpp`
- [ ] `kernel/drivers/net/ethernet.cpp`
- [ ] `kernel/drivers/net/etherdevice.cpp`
- [ ] `kernel/drivers/net/Makefile`
- [ ] `kernel/drivers/Makefile`

### Syscalls
- [ ] `kernel/syscall/syscall.cpp` - System call dispatcher
- [ ] `kernel/syscall/syscall.hpp`
- [ ] `kernel/syscall/syscall_table.cpp` - System call table
- [ ] `kernel/syscall/syscall_table.hpp`
- [ ] `kernel/syscall/entry.asm` - System call entry
- [ ] `kernel/syscall/sys_exit.cpp`
- [ ] `kernel/syscall/sys_fork.cpp`
- [ ] `kernel/syscall/sys_exec.cpp`
- [ ] `kernel/syscall/sys_wait.cpp`
- [ ] `kernel/syscall/sys_open.cpp`
- [ ] `kernel/syscall/sys_read.cpp`
- [ ] `kernel/syscall/sys_write.cpp`
- [ ] `kernel/syscall/sys_close.cpp`
- [ ] `kernel/syscall/sys_mmap.cpp`
- [ ] `kernel/syscall/sys_munmap.cpp`
- [ ] `kernel/syscall/sys_ioctl.cpp`
- [ ] `kernel/syscall/sys_stat.cpp`
- [ ] `kernel/syscall/sys_fstat.cpp`
- [ ] `kernel/syscall/sys_lstat.cpp`
- [ ] `kernel/syscall/sys_access.cpp`
- [ ] `kernel/syscall/sys_chmod.cpp`
- [ ] `kernel/syscall/sys_chown.cpp`
- [ ] `kernel/syscall/sys_link.cpp`
- [ ] `kernel/syscall/sys_unlink.cpp`
- [ ] `kernel/syscall/sys_rename.cpp`
- [ ] `kernel/syscall/sys_mkdir.cpp`
- [ ] `kernel/syscall/sys_rmdir.cpp`
- [ ] `kernel/syscall/sys_dup.cpp`
- [ ] `kernel/syscall/sys_dup2.cpp`
- [ ] `kernel/syscall/sys_pipe.cpp`
- [ ] `kernel/syscall/sys_getpid.cpp`
- [ ] `kernel/syscall/sys_getppid.cpp`
- [ ] `kernel/syscall/sys_getuid.cpp`
- [ ] `kernel/syscall/sys_setuid.cpp`
- [ ] `kernel/syscall/sys_getgid.cpp`
- [ ] `kernel/syscall/sys_setgid.cpp`
- [ ] `kernel/syscall/sys_geteuid.cpp`
- [ ] `kernel/syscall/sys_getegid.cpp`
- [ ] `kernel/syscall/sys_kill.cpp`
- [ ] `kernel/syscall/sys_signal.cpp`
- [ ] `kernel/syscall/sys_sigaction.cpp`
- [ ] `kernel/syscall/sys_sigsuspend.cpp`
- [ ] `kernel/syscall/sys_sigpending.cpp`
- [ ] `kernel/syscall/sys_alarm.cpp`
- [ ] `kernel/syscall/sys_nanosleep.cpp`
- [ ] `kernel/syscall/sys_gettimeofday.cpp`
- [ ] `kernel/syscall/sys_settimeofday.cpp`
- [ ] `kernel/syscall/sys_times.cpp`
- [ ] `kernel/syscall/sys_brk.cpp`
- [ ] `kernel/syscall/sys_sbrk.cpp`
- [ ] `kernel/syscall/sys_clone.cpp`
- [ ] `kernel/syscall/sys_mount.cpp`
- [ ] `kernel/syscall/sys_umount.cpp`
- [ ] `kernel/syscall/sys_sync.cpp`
- [ ] `kernel/syscall/sys_reboot.cpp`
- [ ] `kernel/syscall/sys_sysinfo.cpp`
- [ ] `kernel/syscall/sys_uname.cpp`
- [ ] `kernel/syscall/sys_getcwd.cpp`
- [ ] `kernel/syscall/sys_chdir.cpp`
- [ ] `kernel/syscall/sys_fchdir.cpp`
- [ ] `kernel/syscall/sys_readlink.cpp`
- [ ] `kernel/syscall/sys_getdents.cpp`
- [ ] `kernel/syscall/sys_lseek.cpp`
- [ ] `kernel/syscall/sys_fcntl.cpp`
- [ ] `kernel/syscall/sys_select.cpp`
- [ ] `kernel/syscall/sys_poll.cpp`
- [ ] `kernel/syscall/sys_epoll.cpp`
- [ ] `kernel/syscall/sys_socket.cpp`
- [ ] `kernel/syscall/sys_bind.cpp`
- [ ] `kernel/syscall/sys_listen.cpp`
- [ ] `kernel/syscall/sys_accept.cpp`
- [ ] `kernel/syscall/sys_connect.cpp`
- [ ] `kernel/syscall/sys_sendto.cpp`
- [ ] `kernel/syscall/sys_recvfrom.cpp`
- [ ] `kernel/syscall/sys_shutdown.cpp`
- [ ] `kernel/syscall/sys_setsockopt.cpp`
- [ ] `kernel/syscall/sys_getsockopt.cpp`
- [ ] `kernel/syscall/sys_mprotect.cpp`
- [ ] `kernel/syscall/sys_msync.cpp`
- [ ] `kernel/syscall/sys_mlock.cpp`
- [ ] `kernel/syscall/sys_munlock.cpp`
- [ ] `kernel/syscall/sys_mlockall.cpp`
- [ ] `kernel/syscall/sys_munlockall.cpp`
- [ ] `kernel/syscall/sys_ptrace.cpp`
- [ ] `kernel/syscall/sys_fsync.cpp`
- [ ] `kernel/syscall/sys_fdatasync.cpp`
- [ ] `kernel/syscall/sys_truncate.cpp`
- [ ] `kernel/syscall/sys_ftruncate.cpp`
- [ ] `kernel/syscall/sys_umask.cpp`
- [ ] `kernel/syscall/sys_chroot.cpp`
- [ ] `kernel/syscall/sys_mknod.cpp`
- [ ] `kernel/syscall/sys_symlink.cpp`
- [ ] `kernel/syscall/sys_lseek64.cpp`
- [ ] `kernel/syscall/sys_pread.cpp`
- [ ] `kernel/syscall/sys_pwrite.cpp`
- [ ] `kernel/syscall/sys_getrlimit.cpp`
- [ ] `kernel/syscall/sys_setrlimit.cpp`
- [ ] `kernel/syscall/sys_getrusage.cpp`
- [ ] `kernel/syscall/sys_syslog.cpp`
- [ ] `kernel/syscall/sys_ioperm.cpp`
- [ ] `kernel/syscall/sys_iopl.cpp`
- [ ] `kernel/syscall/Makefile`

### Kernel Library
- [ ] `kernel/lib/string.cpp` - String operations
- [ ] `kernel/lib/string.hpp`
- [ ] `kernel/lib/memory.cpp` - Memory operations
- [ ] `kernel/lib/memory.hpp`
- [ ] `kernel/lib/vsprintf.cpp` - Format printing
- [ ] `kernel/lib/vsprintf.hpp`
- [ ] `kernel/lib/ctype.cpp` - Character classification
- [ ] `kernel/lib/ctype.hpp`
- [ ] `kernel/lib/list.cpp` - Linked lists
- [ ] `kernel/lib/queue.cpp` - Queues
- [ ] `kernel/lib/queue.hpp`
- [ ] `kernel/lib/hash.cpp` - Hash tables
- [ ] `kernel/lib/hash.hpp`
- [ ] `kernel/lib/bitmap.cpp` - Bitmap operations
- [ ] `kernel/lib/bitmap.hpp`
- [ ] `kernel/lib/rbtree.cpp` - Red-black trees
- [ ] `kernel/lib/rbtree.hpp`
- [ ] `kernel/lib/sort.cpp` - Sorting algorithms
- [ ] `kernel/lib/sort.hpp`
- [ ] `kernel/lib/math.cpp` - Math functions
- [ ] `kernel/lib/math.hpp`
- [ ] `kernel/lib/rand.cpp` - Random number generation
- [ ] `kernel/lib/rand.hpp`
- [ ] `kernel/lib/time.cpp` - Time utilities
- [ ] `kernel/lib/time.hpp`
- [ ] `kernel/lib/uuid.cpp` - UUID generation
- [ ] `kernel/lib/uuid.hpp`
- [ ] `kernel/lib/base64.cpp` - Base64 encoding
- [ ] `kernel/lib/base64.hpp`
- [ ] `kernel/lib/printf.cpp` - printf implementation
- [ ] `kernel/lib/kstrtox.cpp` - String to number conversion
- [ ] `kernel/lib/kstrtox.hpp`
- [ ] `kernel/lib/hexdump.cpp` - Hex dump utility
- [ ] `kernel/lib/hexdump.hpp`
- [ ] `kernel/lib/scatterlist.cpp` - Scatter-gather list
- [ ] `kernel/lib/scatterlist.hpp`
- [ ] `kernel/lib/checksum.cpp` - Checksum utilities
- [ ] `kernel/lib/checksum.hpp`
- [ ] `kernel/lib/crc32.cpp` - CRC32 implementation
- [ ] `kernel/lib/crc32.hpp`
- [ ] `kernel/lib/crc16.cpp` - CRC16 implementation
- [ ] `kernel/lib/crc16.hpp`
- [ ] `kernel/lib/crc7.cpp` - CRC7 implementation
- [ ] `kernel/lib/crc7.hpp`
- [ ] `kernel/lib/crc-ccitt.cpp` - CRC-CCITT
- [ ] `kernel/lib/bsearch.cpp` - Binary search
- [ ] `kernel/lib/bsearch.hpp`
- [ ] `kernel/lib/llist.cpp` - Lock-less lists
- [ ] `kernel/lib/llist.hpp`
- [ ] `kernel/lib/kfifo.cpp` - Kernel FIFO
- [ ] `kernel/lib/kfifo.hpp`
- [ ] `kernel/lib/percpu-refcount.cpp` - Per-CPU reference counting
- [ ] `kernel/lib/percpu-refcount.hpp`
- [ ] `kernel/lib/siphash.cpp` - SipHash
- [ ] `kernel/lib/siphash.hpp`
- [ ] `kernel/lib/chacha.cpp` - ChaCha
- [ ] `kernel/lib/chacha.hpp`
- [ ] `kernel/lib/Makefile`
- [ ] `kernel/Makefile`

---

## TODO: USERLAND

### libc (C Library)

#### Include Headers
- [ ] `userland/libc/include/stdio.hpp`
- [ ] `userland/libc/include/stdlib.hpp`
- [ ] `userland/libc/include/string.hpp`
- [ ] `userland/libc/include/unistd.hpp`
- [ ] `userland/libc/include/fcntl.hpp`
- [ ] `userland/libc/include/dirent.hpp`
- [ ] `userland/libc/include/ctype.hpp`
- [ ] `userland/libc/include/time.hpp`
- [ ] `userland/libc/include/signal.hpp`
- [ ] `userland/libc/include/setjmp.hpp`
- [ ] `userland/libc/include/stdint.hpp`
- [ ] `userland/libc/include/stddef.hpp`
- [ ] `userland/libc/include/stdbool.hpp`
- [ ] `userland/libc/include/limits.hpp`
- [ ] `userland/libc/include/float.hpp`
- [ ] `userland/libc/include/math.hpp`
- [ ] `userland/libc/include/assert.hpp`
- [ ] `userland/libc/include/errno.hpp`
- [ ] `userland/libc/include/pwd.hpp`
- [ ] `userland/libc/include/grp.hpp`
- [ ] `userland/libc/include/utsname.hpp`
- [ ] `userland/libc/include/syslog.hpp`
- [ ] `userland/libc/include/termios.hpp`
- [ ] `userland/libc/include/ioctl.hpp`
- [ ] `userland/libc/include/poll.hpp`
- [ ] `userland/libc/include/select.hpp`

##### sys/ Headers
- [ ] `userland/libc/include/sys/types.hpp`
- [ ] `userland/libc/include/sys/stat.hpp`
- [ ] `userland/libc/include/sys/statfs.hpp`
- [ ] `userland/libc/include/sys/wait.hpp`
- [ ] `userland/libc/include/sys/mman.hpp`
- [ ] `userland/libc/include/sys/syscall.hpp`
- [ ] `userland/libc/include/sys/time.hpp`
- [ ] `userland/libc/include/sys/resource.hpp`
- [ ] `userland/libc/include/sys/uio.hpp`
- [ ] `userland/libc/include/sys/socket.hpp`
- [ ] `userland/libc/include/sys/un.hpp`
- [ ] `userland/libc/include/sys/ioctl.hpp`
- [ ] `userland/libc/include/sys/reboot.hpp`

##### bits/ Headers
- [ ] `userland/libc/include/bits/types.hpp`
- [ ] `userland/libc/include/bits/stat.hpp`
- [ ] `userland/libc/include/bits/fcntl.hpp`

#### Source Implementation

##### stdio/
- [ ] `userland/libc/src/stdio/fopen.cpp`
- [ ] `userland/libc/src/stdio/fclose.cpp`
- [ ] `userland/libc/src/stdio/fread.cpp`
- [ ] `userland/libc/src/stdio/fwrite.cpp`
- [ ] `userland/libc/src/stdio/fgetc.cpp`
- [ ] `userland/libc/src/stdio/fputc.cpp`
- [ ] `userland/libc/src/stdio/fgets.cpp`
- [ ] `userland/libc/src/stdio/fputs.cpp`
- [ ] `userland/libc/src/stdio/fprintf.cpp`
- [ ] `userland/libc/src/stdio/fscanf.cpp`
- [ ] `userland/libc/src/stdio/printf.cpp`
- [ ] `userland/libc/src/stdio/scanf.cpp`
- [ ] `userland/libc/src/stdio/sprintf.cpp`
- [ ] `userland/libc/src/stdio/sscanf.cpp`
- [ ] `userland/libc/src/stdio/snprintf.cpp`
- [ ] `userland/libc/src/stdio/vprintf.cpp`
- [ ] `userland/libc/src/stdio/vfprintf.cpp`
- [ ] `userland/libc/src/stdio/vsprintf.cpp`
- [ ] `userland/libc/src/stdio/vsnprintf.cpp`
- [ ] `userland/libc/src/stdio/perror.cpp`
- [ ] `userland/libc/src/stdio/fflush.cpp`
- [ ] `userland/libc/src/stdio/fseek.cpp`
- [ ] `userland/libc/src/stdio/ftell.cpp`
- [ ] `userland/libc/src/stdio/rewind.cpp`
- [ ] `userland/libc/src/stdio/setbuf.cpp`
- [ ] `userland/libc/src/stdio/setvbuf.cpp`
- [ ] `userland/libc/src/stdio/getc.cpp`
- [ ] `userland/libc/src/stdio/putc.cpp`
- [ ] `userland/libc/src/stdio/getchar.cpp`
- [ ] `userland/libc/src/stdio/putchar.cpp`
- [ ] `userland/libc/src/stdio/gets.cpp`
- [ ] `userland/libc/src/stdio/puts.cpp`
- [ ] `userland/libc/src/stdio/ungetc.cpp`
- [ ] `userland/libc/src/stdio/feof.cpp`
- [ ] `userland/libc/src/stdio/ferror.cpp`
- [ ] `userland/libc/src/stdio/clearerr.cpp`
- [ ] `userland/libc/src/stdio/fileno.cpp`
- [ ] `userland/libc/src/stdio/tmpfile.cpp`
- [ ] `userland/libc/src/stdio/tmpnam.cpp`
- [ ] `userland/libc/src/stdio/remove.cpp`
- [ ] `userland/libc/src/stdio/rename.cpp`
- [ ] `userland/libc/src/stdio/Makefile`

##### stdlib/
- [ ] `userland/libc/src/stdlib/malloc.cpp`
- [ ] `userland/libc/src/stdlib/calloc.cpp`
- [ ] `userland/libc/src/stdlib/realloc.cpp`
- [ ] `userland/libc/src/stdlib/free.cpp`
- [ ] `userland/libc/src/stdlib/abort.cpp`
- [ ] `userland/libc/src/stdlib/exit.cpp`
- [ ] `userland/libc/src/stdlib/atexit.cpp`
- [ ] `userland/libc/src/stdlib/system.cpp`
- [ ] `userland/libc/src/stdlib/getenv.cpp`
- [ ] `userland/libc/src/stdlib/putenv.cpp`
- [ ] `userland/libc/src/stdlib/setenv.cpp`
- [ ] `userland/libc/src/stdlib/unsetenv.cpp`
- [ ] `userland/libc/src/stdlib/clearenv.cpp`
- [ ] `userland/libc/src/stdlib/abs.cpp`
- [ ] `userland/libc/src/stdlib/labs.cpp`
- [ ] `userland/libc/src/stdlib/llabs.cpp`
- [ ] `userland/libc/src/stdlib/div.cpp`
- [ ] `userland/libc/src/stdlib/ldiv.cpp`
- [ ] `userland/libc/src/stdlib/lldiv.cpp`
- [ ] `userland/libc/src/stdlib/atoi.cpp`
- [ ] `userland/libc/src/stdlib/atol.cpp`
- [ ] `userland/libc/src/stdlib/atoll.cpp`
- [ ] `userland/libc/src/stdlib/strtol.cpp`
- [ ] `userland/libc/src/stdlib/strtoll.cpp`
- [ ] `userland/libc/src/stdlib/strtoul.cpp`
- [ ] `userland/libc/src/stdlib/strtoull.cpp`
- [ ] `userland/libc/src/stdlib/strtof.cpp`
- [ ] `userland/libc/src/stdlib/strtod.cpp`
- [ ] `userland/libc/src/stdlib/strtold.cpp`
- [ ] `userland/libc/src/stdlib/rand.cpp`
- [ ] `userland/libc/src/stdlib/srand.cpp`
- [ ] `userland/libc/src/stdlib/rand_r.cpp`
- [ ] `userland/libc/src/stdlib/qsort.cpp`
- [ ] `userland/libc/src/stdlib/bsearch.cpp`
- [ ] `userland/libc/src/stdlib/mbtowc.cpp`
- [ ] `userland/libc/src/stdlib/wctomb.cpp`
- [ ] `userland/libc/src/stdlib/mbstowcs.cpp`
- [ ] `userland/libc/src/stdlib/wcstombs.cpp`
- [ ] `userland/libc/src/stdlib/mblen.cpp`
- [ ] `userland/libc/src/stdlib/realpath.cpp`
- [ ] `userland/libc/src/stdlib/mkstemp.cpp`
- [ ] `userland/libc/src/stdlib/mktemp.cpp`
- [ ] `userland/libc/src/stdlib/Makefile`

##### string/
- [ ] `userland/libc/src/string/memcpy.cpp`
- [ ] `userland/libc/src/string/memmove.cpp`
- [ ] `userland/libc/src/string/memset.cpp`
- [ ] `userland/libc/src/string/memcmp.cpp`
- [ ] `userland/libc/src/string/memchr.cpp`
- [ ] `userland/libc/src/string/strcpy.cpp`
- [ ] `userland/libc/src/string/strncpy.cpp`
- [ ] `userland/libc/src/string/strcat.cpp`
- [ ] `userland/libc/src/string/strncat.cpp`
- [ ] `userland/libc/src/string/strcmp.cpp`
- [ ] `userland/libc/src/string/strncmp.cpp`
- [ ] `userland/libc/src/string/strlen.cpp`
- [ ] `userland/libc/src/string/strchr.cpp`
- [ ] `userland/libc/src/string/strrchr.cpp`
- [ ] `userland/libc/src/string/strstr.cpp`
- [ ] `userland/libc/src/string/strpbrk.cpp`
- [ ] `userland/libc/src/string/strspn.cpp`
- [ ] `userland/libc/src/string/strcspn.cpp`
- [ ] `userland/libc/src/string/strtok.cpp`
- [ ] `userland/libc/src/string/strtok_r.cpp`
- [ ] `userland/libc/src/string/strerror.cpp`
- [ ] `userland/libc/src/string/strerror_r.cpp`
- [ ] `userland/libc/src/string/strdup.cpp`
- [ ] `userland/libc/src/string/strndup.cpp`
- [ ] `userland/libc/src/string/strcasecmp.cpp`
- [ ] `userland/libc/src/string/strncasecmp.cpp`
- [ ] `userland/libc/src/string/strcoll.cpp`
- [ ] `userland/libc/src/string/strxfrm.cpp`
- [ ] `userland/libc/src/string/Makefile`

##### unistd/
- [ ] `userland/libc/src/unistd/access.cpp`
- [ ] `userland/libc/src/unistd/alarm.cpp`
- [ ] `userland/libc/src/unistd/brk.cpp`
- [ ] `userland/libc/src/unistd/chdir.cpp`
- [ ] `userland/libc/src/unistd/chown.cpp`
- [ ] `userland/libc/src/unistd/close.cpp`
- [ ] `userland/libc/src/unistd/dup.cpp`
- [ ] `userland/libc/src/unistd/dup2.cpp`
- [ ] `userland/libc/src/unistd/execl.cpp`
- [ ] `userland/libc/src/unistd/execv.cpp`
- [ ] `userland/libc/src/unistd/execve.cpp`
- [ ] `userland/libc/src/unistd/execvp.cpp`
- [ ] `userland/libc/src/unistd/_exit.cpp`
- [ ] `userland/libc/src/unistd/fork.cpp`
- [ ] `userland/libc/src/unistd/getcwd.cpp`
- [ ] `userland/libc/src/unistd/getegid.cpp`
- [ ] `userland/libc/src/unistd/geteuid.cpp`
- [ ] `userland/libc/src/unistd/getgid.cpp`
- [ ] `userland/libc/src/unistd/getgroups.cpp`
- [ ] `userland/libc/src/unistd/getlogin.cpp`
- [ ] `userland/libc/src/unistd/getpgid.cpp`
- [ ] `userland/libc/src/unistd/getpgrp.cpp`
- [ ] `userland/libc/src/unistd/getpid.cpp`
- [ ] `userland/libc/src/unistd/getppid.cpp`
- [ ] `userland/libc/src/unistd/getuid.cpp`
- [ ] `userland/libc/src/unistd/isatty.cpp`
- [ ] `userland/libc/src/unistd/link.cpp`
- [ ] `userland/libc/src/unistd/lseek.cpp`
- [ ] `userland/libc/src/unistd/pipe.cpp`
- [ ] `userland/libc/src/unistd/read.cpp`
- [ ] `userland/libc/src/unistd/rmdir.cpp`
- [ ] `userland/libc/src/unistd/setgid.cpp`
- [ ] `userland/libc/src/unistd/setpgid.cpp`
- [ ] `userland/libc/src/unistd/setsid.cpp`
- [ ] `userland/libc/src/unistd/setuid.cpp`
- [ ] `userland/libc/src/unistd/sleep.cpp`
- [ ] `userland/libc/src/unistd/symlink.cpp`
- [ ] `userland/libc/src/unistd/sysconf.cpp`
- [ ] `userland/libc/src/unistd/tcgetpgrp.cpp`
- [ ] `userland/libc/src/unistd/tcsetpgrp.cpp`
- [ ] `userland/libc/src/unistd/ttyname.cpp`
- [ ] `userland/libc/src/unistd/ualarm.cpp`
- [ ] `userland/libc/src/unistd/unlink.cpp`
- [ ] `userland/libc/src/unistd/usleep.cpp`
- [ ] `userland/libc/src/unistd/write.cpp`
- [ ] `userland/libc/src/unistd/Makefile`

##### syscall/
- [ ] `userland/libc/src/syscall/syscall.cpp` - Syscall wrapper
- [ ] `userland/libc/src/syscall/syscall.S` - Assembly syscall entry
- [ ] `userland/libc/src/syscall/Makefile`

##### ctype/
- [ ] `userland/libc/src/ctype/isalnum.cpp`
- [ ] `userland/libc/src/ctype/isalpha.cpp`
- [ ] `userland/libc/src/ctype/isascii.cpp`
- [ ] `userland/libc/src/ctype/isblank.cpp`
- [ ] `userland/libc/src/ctype/iscntrl.cpp`
- [ ] `userland/libc/src/ctype/isdigit.cpp`
- [ ] `userland/libc/src/ctype/isgraph.cpp`
- [ ] `userland/libc/src/ctype/islower.cpp`
- [ ] `userland/libc/src/ctype/isprint.cpp`
- [ ] `userland/libc/src/ctype/ispunct.cpp`
- [ ] `userland/libc/src/ctype/isspace.cpp`
- [ ] `userland/libc/src/ctype/isupper.cpp`
- [ ] `userland/libc/src/ctype/isxdigit.cpp`
- [ ] `userland/libc/src/ctype/tolower.cpp`
- [ ] `userland/libc/src/ctype/toupper.cpp`
- [ ] `userland/libc/src/ctype/Makefile`

#### Library & Startup
- [ ] `userland/libc/src/crt0.asm` - C runtime startup
- [ ] `userland/libc/lib/libc.a` - Static library (generated)
- [ ] `userland/libc/Makefile`

### Shell
- [ ] `userland/shell/src/main.cpp`
- [ ] `userland/shell/src/parser.cpp`
- [ ] `userland/shell/src/parser.hpp`
- [ ] `userland/shell/src/lexer.cpp`
- [ ] `userland/shell/src/lexer.hpp`
- [ ] `userland/shell/src/builtin.cpp` - Built-in commands
- [ ] `userland/shell/src/builtin.hpp`
- [ ] `userland/shell/src/exec.cpp`
- [ ] `userland/shell/src/exec.hpp`
- [ ] `userland/shell/src/job.cpp` - Job control
- [ ] `userland/shell/src/job.hpp`
- [ ] `userland/shell/src/env.cpp` - Environment variables
- [ ] `userland/shell/src/env.hpp`
- [ ] `userland/shell/src/history.cpp` - Command history
- [ ] `userland/shell/src/history.hpp`
- [ ] `userland/shell/src/prompt.cpp` - Shell prompt
- [ ] `userland/shell/src/prompt.hpp`
- [ ] `userland/shell/src/signal.cpp` - Signal handling
- [ ] `userland/shell/src/signal.hpp`
- [ ] `userland/shell/src/redirection.cpp` - I/O redirection
- [ ] `userland/shell/src/redirection.hpp`
- [ ] `userland/shell/src/pipe.cpp` - Pipe handling
- [ ] `userland/shell/src/pipe.hpp`
- [ ] `userland/shell/src/wildcard.cpp` - Glob patterns
- [ ] `userland/shell/src/wildcard.hpp`
- [ ] `userland/shell/src/variables.cpp` - Shell variables
- [ ] `userland/shell/src/variables.hpp`
- [ ] `userland/shell/src/complete.cpp` - Tab completion
- [ ] `userland/shell/src/complete.hpp`
- [ ] `userland/shell/include/shell.hpp`
- [ ] `userland/shell/Makefile`

### User Binaries (bin/)
- [ ] `userland/bin/init/init.cpp` - Init process
- [ ] `userland/bin/init/Makefile`
- [ ] `userland/bin/ls/ls.cpp`
- [ ] `userland/bin/ls/Makefile`
- [ ] `userland/bin/cat/cat.cpp`
- [ ] `userland/bin/cat/Makefile`
- [ ] `userland/bin/echo/echo.cpp`
- [ ] `userland/bin/echo/Makefile`
- [ ] `userland/bin/mkdir/mkdir.cpp`
- [ ] `userland/bin/mkdir/Makefile`
- [ ] `userland/bin/rm/rm.cpp`
- [ ] `userland/bin/rm/Makefile`
- [ ] `userland/bin/cp/cp.cpp`
- [ ] `userland/bin/cp/Makefile`
- [ ] `userland/bin/mv/mv.cpp`
- [ ] `userland/bin/mv/Makefile`
- [ ] `userland/bin/ps/ps.cpp`
- [ ] `userland/bin/ps/Makefile`
- [ ] `userland/bin/kill/kill.cpp`
- [ ] `userland/bin/kill/Makefile`
- [ ] `userland/bin/clear/clear.cpp`
- [ ] `userland/bin/clear/Makefile`
- [ ] `userland/bin/date/date.cpp`
- [ ] `userland/bin/date/Makefile`
- [ ] `userland/bin/pwd/pwd.cpp`
- [ ] `userland/bin/pwd/Makefile`
- [ ] `userland/bin/cd/Makefile` - Built into shell
- [ ] `userland/bin/mount/mount.cpp`
- [ ] `userland/bin/mount/Makefile`
- [ ] `userland/bin/umount/umount.cpp`
- [ ] `userland/bin/umount/Makefile`
- [ ] `userland/bin/df/df.cpp`
- [ ] `userland/bin/df/Makefile`
- [ ] `userland/bin/du/du.cpp`
- [ ] `userland/bin/du/Makefile`
- [ ] `userland/bin/grep/grep.cpp`
- [ ] `userland/bin/grep/Makefile`
- [ ] `userland/bin/test/test.cpp`
- [ ] `userland/bin/test/Makefile`
- [ ] `userland/bin/chmod/chmod.cpp`
- [ ] `userland/bin/chmod/Makefile`
- [ ] `userland/bin/chown/chown.cpp`
- [ ] `userland/bin/chown/Makefile`
- [ ] `userland/bin/ln/ln.cpp`
- [ ] `userland/bin/ln/Makefile`
- [ ] `userland/bin/touch/touch.cpp`
- [ ] `userland/bin/touch/Makefile`
- [ ] `userland/bin/head/head.cpp`
- [ ] `userland/bin/head/Makefile`
- [ ] `userland/bin/tail/tail.cpp`
- [ ] `userland/bin/tail/Makefile`
- [ ] `userland/bin/wc/wc.cpp`
- [ ] `userland/bin/wc/Makefile`
- [ ] `userland/bin/sort/sort.cpp`
- [ ] `userland/bin/sort/Makefile`
- [ ] `userland/bin/uniq/uniq.cpp`
- [ ] `userland/bin/uniq/Makefile`
- [ ] `userland/bin/tr/tr.cpp`
- [ ] `userland/bin/tr/Makefile`
- [ ] `userland/bin/tee/tee.cpp`
- [ ] `userland/bin/tee/Makefile`
- [ ] `userland/bin/more/more.cpp`
- [ ] `userland/bin/more/Makefile`
- [ ] `userland/bin/less/less.cpp`
- [ ] `userland/bin/less/Makefile`
- [ ] `userland/bin/find/find.cpp`
- [ ] `userland/bin/find/Makefile`
- [ ] `userland/bin/xargs/xargs.cpp`
- [ ] `userland/bin/xargs/Makefile`
- [ ] `userland/bin/whoami/whoami.cpp`
- [ ] `userland/bin/whoami/Makefile`
- [ ] `userland/bin/id/id.cpp`
- [ ] `userland/bin/id/Makefile`
- [ ] `userland/bin/hostname/hostname.cpp`
- [ ] `userland/bin/hostname/Makefile`
- [ ] `userland/bin/uname/uname.cpp`
- [ ] `userland/bin/uname/Makefile`
- [ ] `userland/bin/uptime/uptime.cpp`
- [ ] `userland/bin/uptime/Makefile`
- [ ] `userland/bin/sleep/sleep.cpp`
- [ ] `userland/bin/sleep/Makefile`
- [ ] `userland/bin/sync/sync.cpp`
- [ ] `userland/bin/sync/Makefile`
- [ ] `userland/bin/reboot/reboot.cpp`
- [ ] `userland/bin/reboot/Makefile`
- [ ] `userland/bin/Makefile`

### System Binaries (sbin/)
- [ ] `userland/sbin/login/login.cpp`
- [ ] `userland/sbin/login/Makefile`
- [ ] `userland/sbin/getty/getty.cpp`
- [ ] `userland/sbin/getty/Makefile`
- [ ] `userland/sbin/fdisk/fdisk.cpp`
- [ ] `userland/sbin/fdisk/Makefile`
- [ ] `userland/sbin/mkfs/mkfs.cpp`
- [ ] `userland/sbin/mkfs/mkfs_ext2.cpp`
- [ ] `userland/sbin/mkfs/mkfs_fat.cpp`
- [ ] `userland/sbin/mkfs/Makefile`
- [ ] `userland/sbin/fsck/fsck.cpp`
- [ ] `userland/sbin/fsck/fsck_ext2.cpp`
- [ ] `userland/sbin/fsck/Makefile`
- [ ] `userland/sbin/ifconfig/ifconfig.cpp`
- [ ] `userland/sbin/ifconfig/Makefile`
- [ ] `userland/sbin/route/route.cpp`
- [ ] `userland/sbin/route/Makefile`
- [ ] `userland/sbin/ping/ping.cpp`
- [ ] `userland/sbin/ping/Makefile`
- [ ] `userland/sbin/dhcp/dhcp.cpp`
- [ ] `userland/sbin/dhcp/Makefile`
- [ ] `userland/sbin/Makefile`

### Libraries (lib/)
- [ ] `userland/lib/ld.so` - Dynamic linker
- [ ] `userland/lib/libc.so` - Shared C library
- [ ] `userland/lib/libm.so` - Math library
- [ ] `userland/lib/Makefile`

### Configuration (etc/)
- [ ] `userland/etc/passwd`
- [ ] `userland/etc/shadow`
- [ ] `userland/etc/group`
- [ ] `userland/etc/motd`
- [ ] `userland/etc/fstab`
- [ ] `userland/etc/hostname`
- [ ] `userland/etc/hosts`
- [ ] `userland/etc/resolv.conf`
- [ ] `userland/etc/inittab`
- [ ] `userland/etc/profile`
- [ ] `userland/etc/shells`
- [ ] `userland/etc/termcap`
- [ ] `userland/etc/timezone`
- [ ] `userland/etc/services`
- [ ] `userland/etc/protocols`
- [ ] `userland/etc/networks`
- [ ] `userland/etc/Makefile`

### Share (share/)
- [ ] `userland/share/doc/` - Documentation directory
- [ ] `userland/share/man/` - Man pages directory
- [ ] `userland/share/locale/` - Localization directory
- [ ] `userland/share/Makefile`
- [ ] `userland/Makefile`

---

## TODO: INITRD (Initial RAM Disk)
- [ ] `initrd/init` - Init script
- [ ] `initrd/bin/` - Essential binaries (links to userland)
- [ ] `initrd/sbin/` - System binaries
- [ ] `initrd/lib/` - Shared libraries
- [ ] `initrd/etc/inittab`
- [ ] `initrd/etc/fstab.initrd`
- [ ] `initrd/etc/profile`
- [ ] `initrd/dev/` - Device files (created at runtime)
- [ ] `initrd/proc/` - Proc mount point
- [ ] `initrd/sys/` - Sys mount point
- [ ] `initrd/tmp/` - Temp directory
- [ ] `initrd/Makefile`

---

## TODO: TOOLS

### Cross-Compiler
- [ ] `tools/cross-compiler/build-binutils.sh`
- [ ] `tools/cross-compiler/build-gcc.sh`
- [ ] `tools/cross-compiler/build-gdb.sh`
- [ ] `tools/cross-compiler/download.sh`
- [ ] `tools/cross-compiler/config.sh`
- [ ] `tools/cross-compiler/patches/` - Patch directory
- [ ] `tools/cross-compiler/README.md`

### Build ISO
- [ ] `tools/build-iso/mkiso.sh`
- [ ] `tools/build-iso/mkiso.py`
- [ ] `tools/build-iso/isohybrid.cpp` - Make ISO bootable from USB
- [ ] `tools/build-iso/Makefile`

### Initrd Creation
- [ ] `tools/mkinitrd/mkinitrd.cpp`
- [ ] `tools/mkinitrd/mkinitrd.sh`
- [ ] `tools/mkinitrd/cpio.cpp` - CPIO archive format
- [ ] `tools/mkinitrd/Makefile`

### Font Converter
- [ ] `tools/font-converter/fontconv.cpp`
- [ ] `tools/font-converter/psf2c.cpp` - PSF to C array
- [ ] `tools/font-converter/bdf2c.cpp` - BDF to C array
- [ ] `tools/font-converter/Makefile`

### Debug Tools
- [ ] `tools/debug/gdb-init` - GDB configuration
- [ ] `tools/debug/bochsrc` - Bochs emulator config
- [ ] `tools/debug/bochs-debugger.sh`
- [ ] `tools/debug/qemu-gdb.sh`
- [ ] `tools/debug/trace.py` - Kernel tracing script
- [ ] `tools/debug/README.md`

### Kconfig (Kernel Config)
- [ ] `tools/kconfig/mconf.cpp` - Menuconfig
- [ ] `tools/kconfig/conf.cpp` - Config
- [ ] `tools/kconfig/qconf.cpp` - Qt config (optional)
- [ ] `tools/kconfig/gconf.cpp` - GTK config (optional)
- [ ] `tools/kconfig/Makefile`
- [ ] `tools/Makefile`

---

## TODO: DOCS

### Architecture
- [ ] `docs/architecture/overview.md`
- [ ] `docs/architecture/boot-process.md`
- [ ] `docs/architecture/memory-management.md`
- [ ] `docs/architecture/process-scheduling.md`
- [ ] `docs/architecture/filesystem.md`
- [ ] `docs/architecture/networking.md`
- [ ] `docs/architecture/drivers.md`
- [ ] `docs/architecture/smp.md`

### API Documentation
- [ ] `docs/api/kernel-api.md`
- [ ] `docs/api/syscalls.md`
- [ ] `docs/api/vfs.md`
- [ ] `docs/api/driver-api.md`

### Driver Development
- [ ] `docs/drivers/writing-drivers.md`
- [ ] `docs/drivers/pci.md`
- [ ] `docs/drivers/usb.md`
- [ ] `docs/drivers/network.md`

### Boot Documentation
- [ ] `docs/boot/bootloader.md`
- [ ] `docs/boot/multiboot.md`
- [ ] `docs/boot/grub-setup.md`

### Porting Guide
- [ ] `docs/porting/porting-guide.md`
- [ ] `docs/porting/new-architecture.md`

### User Guide
- [ ] `docs/user/installation.md`
- [ ] `docs/user/virtualbox.md`
- [ ] `docs/user/qemu.md`
- [ ] `docs/user/getting-started.md`

### Development
- [ ] `docs/development/coding-style.md`
- [ ] `docs/development/debugging.md`
- [ ] `docs/development/testing.md`
- [ ] `docs/development/contributing.md`
- [ ] `docs/README.md`
- [ ] `docs/Makefile`

---

## TODO: TESTS

### Unit Tests
- [ ] `tests/unit/mm/test_pmm.cpp`
- [ ] `tests/unit/mm/test_vmm.cpp`
- [ ] `tests/unit/mm/test_slab.cpp`
- [ ] `tests/unit/mm/Makefile`
- [ ] `tests/unit/sched/test_sched.cpp`
- [ ] `tests/unit/sched/test_mutex.cpp`
- [ ] `tests/unit/sched/Makefile`
- [ ] `tests/unit/fs/test_vfs.cpp`
- [ ] `tests/unit/fs/test_ext2.cpp`
- [ ] `tests/unit/fs/Makefile`
- [ ] `tests/unit/lib/test_string.cpp`
- [ ] `tests/unit/lib/test_list.cpp`
- [ ] `tests/unit/lib/Makefile`
- [ ] `tests/unit/Makefile`

### Integration Tests
- [ ] `tests/integration/boot/test_boot.sh`
- [ ] `tests/integration/boot/test_multiboot.sh`
- [ ] `tests/integration/syscall/test_syscalls.cpp`
- [ ] `tests/integration/syscall/Makefile`
- [ ] `tests/integration/drivers/test_pci.sh`
- [ ] `tests/integration/drivers/Makefile`
- [ ] `tests/integration/fs/test_mount.sh`
- [ ] `tests/integration/fs/Makefile`
- [ ] `tests/integration/Makefile`

### Kernel Unit Tests
- [ ] `tests/kernel/kunit/kunit.cpp`
- [ ] `tests/kernel/kunit/kunit.hpp`
- [ ] `tests/kernel/kunit/Makefile`
- [ ] `tests/kernel/test_main.cpp`
- [ ] `tests/kernel/Makefile`

### Test Scripts
- [ ] `tests/scripts/run-tests.sh`
- [ ] `tests/scripts/test-runner.py`
- [ ] `tests/scripts/coverage.sh`
- [ ] `tests/Makefile`

---

## TODO: SCRIPTS
- [ ] `scripts/build.sh` - Main build script
- [ ] `scripts/build-kernel.sh` - Build kernel only
- [ ] `scripts/build-userland.sh` - Build userland only
- [ ] `scripts/clean.sh` - Clean build
- [ ] `scripts/config.sh` - Configure build
- [ ] `scripts/menuconfig.sh` - Interactive config
- [ ] `scripts/run-qemu.sh` - Run in QEMU
- [ ] `scripts/run-qemu-bios.sh` - BIOS boot
- [ ] `scripts/run-qemu-uefi.sh` - UEFI boot
- [ ] `scripts/run-qemu-debug.sh` - Debug mode
- [ ] `scripts/run-vbox.sh` - Run in VirtualBox
- [ ] `scripts/create-vm.sh` - Create VM
- [ ] `scripts/start-vm.sh` - Start VM
- [ ] `scripts/stop-vm.sh` - Stop VM
- [ ] `scripts/delete-vm.sh` - Delete VM
- [ ] `scripts/run-bochs.sh` - Run in Bochs
- [ ] `scripts/create-iso.sh` - ISO creation
- [x] `scripts/create-disk.sh` - Create disk image (stage1, stage2, loader, kernel)
- [ ] `scripts/install-toolchain.sh` - Install cross-compiler
- [ ] `scripts/check-toolchain.sh` - Verify toolchain
- [ ] `scripts/debug.sh` - Debug script
- [ ] `scripts/gdb.sh` - GDB wrapper
- [ ] `scripts/symbolize.sh` - Symbolize stack traces
- [ ] `scripts/checkpatch.pl` - Check coding style
- [ ] `scripts/checkstack.pl` - Check stack usage
- [ ] `scripts/tags.sh` - Generate ctags
- [ ] `scripts/cscope.sh` - Generate cscope
- [ ] `scripts/Makefile`

---

## TODO: CONFIGURATION
- [ ] `config/Kconfig` - Top-level Kconfig
- [ ] `config/Kconfig.debug`
- [ ] `config/Kconfig.mm`
- [ ] `config/Kconfig.sched`
- [ ] `config/Kconfig.fs`
- [ ] `config/Kconfig.net`
- [ ] `config/Kconfig.drivers`
- [ ] `config/kernel.config` - Kernel configuration
- [ ] `config/kernel.config.default` - Default config
- [ ] `config/kernel.config.debug` - Debug config
- [ ] `config/kernel.config.release` - Release config
- [ ] `config/build.config` - Build configuration
- [ ] `config/toolchain.config` - Toolchain settings
- [ ] `config/Makefile.common` - Common Makefile rules
- [ ] `config/arch/x86_64/Kconfig`
- [ ] `config/Makefile`

---

## TODO: PROJECT ROOT FILES
- [ ] `Kbuild` - Kernel build system
- [ ] `Kconfig` - Kernel configuration menu
- [ ] `LICENSE` - License file
- [ ] `CONTRIBUTING.md` - Contribution guidelines
- [ ] `AUTHORS` - List of contributors
- [ ] `.gitignore` - Git ignore rules
- [ ] `.gitattributes` - Git attributes
- [ ] `.clang-format` - Clang format config
- [ ] `.editorconfig` - Editor configuration
- [ ] `Doxyfile` - Doxygen documentation config

---

## TODO: BUILD OUTPUT (Generated)
- [ ] `build/boot/stage1.bin`
- [ ] `build/boot/stage2.bin`
- [ ] `build/kernel/kernel.bin` - Kernel executable (ELF)
- [ ] `build/kernel/kernel.sym` - Symbol table
- [ ] `build/kernel/kernel.map` - Linker map
- [ ] `build/kernel/*.o` - Object files
- [ ] `build/userland/libc.a` - C library (static)
- [ ] `build/userland/libc.so` - C library (shared)
- [ ] `build/userland/bin/` - User binaries
- [ ] `build/userland/sbin/` - System binaries
- [ ] `build/userland/lib/` - Shared libraries
- [ ] `build/iso/DIT-OS.iso` - Final bootable ISO

---

## TODO: ISO STRUCTURE
- [ ] `iso/boot/grub/grub.cfg`
- [ ] `iso/boot/kernel.bin`
- [ ] `iso/boot/limine/` - If using Limine
- [ ] `iso/initrd.img`
- [ ] `iso/limine-bios-cd.bin`
- [ ] `iso/limine-uefi-cd.bin`
- [ ] `iso/EFI/BOOT/BOOTX64.EFI`
- [ ] `iso/Makefile`

---

## TODO: GITHUB / CI/CD
- [ ] `.github/workflows/build.yml` - CI/CD build workflow
- [ ] `.github/workflows/test.yml` - Test workflow
- [ ] `.github/workflows/release.yml` - Release workflow
- [ ] `.github/ISSUE_TEMPLATE/bug_report.md`
- [ ] `.github/ISSUE_TEMPLATE/feature_request.md`
- [ ] `.github/PULL_REQUEST_TEMPLATE.md`

---

## TODO: IDE SUPPORT
- [ ] `.vscode/settings.json` - VS Code settings
- [ ] `.vscode/tasks.json` - Build tasks
- [ ] `.vscode/launch.json` - Debug configuration

---

## STATISTICS

**Estimated Total Files**: ~1,500-2,000  
**Currently Completed**: 131 files (~7%)  
**Remaining**: ~1,369-1,869 files

### By Category:
- **Source Files** (.cpp, .hpp, .c, .h): ~800-1,000 needed
- **Assembly Files** (.asm, .S): ~50-80 needed  
- **Build/Make Files**: ~100-150 needed
- **Documentation**: ~50-80 needed
- **Configuration**: ~30-50 needed
- **Scripts**: ~40-60 needed
- **Test Files**: ~100-150 needed

---

## NEXT STEPS (Recommended Priority)

1. **Boot System** - GRUB theme optional; custom bootloader is primary and working
2. **Kernel Core** - Entry point, GDT/IDT/TSS/PIC, VGA print, interrupt handler done
3. **Memory Management** - PMM, VMM, heap, paging, TLB, DMA stubs done
4. **Build System** - Kernel Makefile builds kernel.bin; bootloader builds disk image
5. **Device drivers** - Serial, keyboard, mouse, timer, RTC, PCI, ATA, AHCI, NVMe, floppy done
6. **Userland libc** - Basic C library for user programs
7. **Shell** - Basic command interpreter
8. **Filesystem** - RAM filesystem (ramfs) for testing

---

*Last Updated: 2026-02-22*  
*Based on PROJECT_STRUCTURE.md*
