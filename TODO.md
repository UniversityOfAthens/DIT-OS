# DIT-OS Project TODO

Comprehensive task list for building the DIT Operating System based on PROJECT_STRUCTURE.md.

**Current Progress: ~12-16% complete** (248 of ~1,500-2,000 estimated files)

**Boot path:** We use the **custom bootloader** (stage1 + stage2). GRUB is **not** used for booting; GRUB/boot/ entries below are optional (e.g. for ISO or alternate setups).

---

## COMPLETED (248 files)

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
- [ ] `scripts/clean.sh` - Clean build artifacts (bootloader/build/, kernel/build/, build/)
- [ ] `scripts/config.sh` - Configure build options
- [ ] `scripts/menuconfig.sh` - Interactive config (wraps tools/kconfig)
- [x] `scripts/run-qemu.sh` - Run in QEMU (custom bootloader disk/floppy)
- [ ] `scripts/run-qemu-bios.sh` - QEMU BIOS boot
- [ ] `scripts/run-qemu-uefi.sh` - QEMU UEFI boot
- [ ] `scripts/run-qemu-debug.sh` - QEMU debug mode (add -s -S flags, wait for GDB)
- [ ] `scripts/run-vbox.sh` - Run in VirtualBox
- [ ] `scripts/run-vbox/create-vm.sh` - Create VM
- [ ] `scripts/run-vbox/start-vm.sh` - Start VM
- [ ] `scripts/run-vbox/stop-vm.sh` - Stop VM
- [ ] `scripts/run-vbox/delete-vm.sh` - Delete VM
- [ ] `scripts/run-bochs.sh` - Run in Bochs
- [ ] `scripts/create-iso.sh` - ISO creation (uses custom bootloader, not GRUB)
- [x] `scripts/create-disk.sh` - Create disk image (stage1, stage2, loader, kernel)
- [ ] `scripts/install-toolchain.sh` - Install cross-compiler (calls tools/cross-compiler scripts)
- [ ] `scripts/check-toolchain.sh` - Verify toolchain (i686-elf-gcc, nasm, ld)
- [ ] `scripts/debug.sh` - Debug script (launch QEMU + GDB together)
- [ ] `scripts/gdb.sh` - GDB wrapper (load kernel.sym, connect to :1234)
- [ ] `scripts/symbolize.sh` - Symbolize stack traces using kernel.sym
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
- [x] `kernel/arch/x86_64/graphics/vga.cpp` - VGA text mode
- [ ] `kernel/arch/x86_64/graphics/vga.hpp`
- [ ] `kernel/arch/x86_64/graphics/vbe.cpp` - VESA BIOS Extensions
- [ ] `kernel/arch/x86_64/graphics/vbe.hpp`
- [x] `kernel/arch/x86_64/graphics/framebuffer.cpp` - Linear framebuffer
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
- [x] `kernel/include/dev/device.hpp`
- [x] `kernel/include/dev/driver.hpp`
- [x] `kernel/include/dev/bus.hpp`
- [x] `kernel/include/dev/class.hpp`
- [x] `kernel/include/dev/serial.hpp`
- [x] `kernel/include/dev/keyboard.hpp`
- [x] `kernel/include/dev/keycodes.hpp`
- [x] `kernel/include/dev/timer.hpp`
- [x] `kernel/include/dev/rtc.hpp`
- [x] `kernel/include/dev/pci.hpp`
- [x] `kernel/include/dev/pci_ids.hpp`
- [x] `kernel/include/dev/ata.hpp`
- [x] `kernel/include/dev/usb.hpp`
- [x] `kernel/include/dev/usb_spec.hpp`
- [x] `kernel/include/dev/blockdev.hpp`
- [x] `kernel/include/dev/chardev.hpp`

#### Filesystem Headers
- [x] `kernel/include/fs/vfs.hpp`
- [x] `kernel/include/fs/inode.hpp`
- [x] `kernel/include/fs/dentry.hpp`
- [x] `kernel/include/fs/superblock.hpp`
- [x] `kernel/include/fs/file.hpp`
- [x] `kernel/include/fs/mount.hpp`
- [x] `kernel/include/fs/fs.hpp`
- [x] `kernel/include/fs/namei.hpp`
- [x] `kernel/include/fs/stat.hpp`
- [x] `kernel/include/fs/fcntl.hpp`
- [x] `kernel/include/fs/ioctl.hpp`
- [x] `kernel/include/fs/dirent.hpp`
- [x] `kernel/include/fs/poll.hpp`
- [x] `kernel/include/fs/select.hpp`
- [x] `kernel/include/fs/buffer.hpp`

#### Graphics Headers
- [x] `kernel/include/graphics/vga.hpp`
- [x] `kernel/include/graphics/framebuffer.hpp`
- [x] `kernel/include/graphics/font.hpp`
- [x] `kernel/include/graphics/color.hpp`
- [x] `kernel/include/graphics/drawing.hpp`
- [x] `kernel/include/graphics/cursor.hpp`
- [x] `kernel/include/graphics/terminal.hpp`
- [x] `kernel/include/graphics/terminal-buffer.hpp`

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
- [x] `kernel/include/syscall/syscall.hpp`
- [x] `kernel/include/syscall/syscall_table.hpp`
- [x] `kernel/include/syscall/syscall_numbers.hpp`
- [x] `kernel/include/syscall/syscall_defs.hpp`
- [x] `kernel/include/syscall/syscall_trace.hpp`

#### Scheduler Headers
- [x] `kernel/include/sched/sched.hpp` - Scheduler header (exists)
- [x] `kernel/include/sched/task.hpp` - Task struct header (exists)
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

> **⚠ NOTE FOR CURSOR**: `kernel/sched/sched.cpp`, `task.cpp`, `shell.cpp`, `init_task.cpp`, `idle.cpp` already exist and are partially implemented. Do NOT recreate them from scratch — only fill in missing pieces. The existing sched uses `sched/sched.hpp` and `sched/task.hpp` headers from `kernel/include/`. This must stay consistent.

- [x] `kernel/sched/sched.cpp` - Round-robin scheduler (implemented, basic)
- [x] `kernel/sched/task.cpp` - Task creation (implemented)
- [x] `kernel/sched/shell.cpp` - Init/shell task (implemented, minimal)
- [x] `kernel/sched/init_task.cpp` - Init task entry (implemented)
- [x] `kernel/sched/idle.cpp` - Idle task (implemented)
- [x] `kernel/sched/context.asm` - Context switch (implemented, uses `sched_switch`)
- [x] `kernel/include/sched/sched.hpp` - Scheduler header (exists, used by all sched/*.cpp)
- [x] `kernel/include/sched/task.hpp` - Task struct header (exists)
- [x] `kernel/sched/fork.cpp` - Process creation (clone task, copy address space)
- [ ] `kernel/sched/exec.cpp` - Program execution (load ELF into process)
- [x] `kernel/sched/exit.cpp` - Process termination (cleanup, notify parent)
- [ ] `kernel/sched/wait.cpp` - Wait for child processes (waitpid)
- [ ] `kernel/sched/signal.cpp` - Signal handling (kill, sigaction, etc.)
- [ ] `kernel/sched/signal.hpp`
- [ ] `kernel/sched/timer.cpp` - Per-task timers and sleeping (sleep, nanosleep)
- [ ] `kernel/sched/entry.asm` - Task entry points (usermode return trampoline)
- [x] `kernel/sched/mutex.cpp` - Mutex implementation
- [ ] `kernel/sched/semaphore.cpp` - Semaphore implementation
- [ ] `kernel/sched/rwlock.cpp` - Read-write locks
- [ ] `kernel/sched/condvar.cpp` - Condition variables
- [ ] `kernel/sched/futex.cpp` - Fast userspace mutex
- [ ] `kernel/sched/process.cpp` - Process management (address space, pid table)
- [ ] `kernel/sched/thread.cpp` - Thread management (kernel threads)
- [x] `kernel/sched/pid.cpp` - PID allocation (bitmap or radix tree)
- [ ] `kernel/sched/loadavg.cpp` - Load average calculation
- [ ] `kernel/sched/clock.cpp` - Scheduling clock
- [ ] `kernel/sched/deadline.cpp` - Deadline scheduler
- [ ] `kernel/sched/fair.cpp` - CFS scheduler (replace round-robin eventually)
- [ ] `kernel/sched/rt.cpp` - Real-time scheduler
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
- [ ] `kernel/sched/sched_api.hpp` - Public scheduler API (expose run_queue_add, task_create, etc.)
- [ ] `kernel/sched/Makefile` - **⚠ Must include existing files: sched.cpp, task.cpp, shell.cpp, init_task.cpp, idle.cpp, context.asm**

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

#### ramfs (RAM Filesystem) — **Priority: implement first for userland testing**
- [x] `kernel/fs/ramfs/ramfs.cpp`
- [x] `kernel/fs/ramfs/ramfs.hpp`
- [x] `kernel/fs/ramfs/inode.cpp`
- [x] `kernel/fs/ramfs/Makefile`

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

### Syscall Layer

> **NOTE FOR CURSOR**: Syscall numbers must be consistent between `userland/include/syscall.h` and `kernel/include/syscall/syscall_numbers.hpp` (e.g. `SYS_exit = 6`). The syscall ABI is: `int 0x80`, EAX=number, return in EAX (negative = -errno).

- [x] `kernel/syscall/syscall.cpp` - Syscall dispatcher (int 0x80 handler → table lookup)
- [ ] `kernel/syscall/syscall.hpp` - **⚠ Must match `kernel/include/syscall/syscall.hpp`; do not duplicate — pick one location**
- [ ] `kernel/syscall/syscall_table.cpp` - Syscall function pointer table
- [ ] `kernel/syscall/sys_read.cpp` - read() syscall
- [ ] `kernel/syscall/sys_write.cpp` - write() syscall
- [x] `kernel/syscall/sys_exit.cpp` - exit() syscall
- [x] `kernel/syscall/sys_fork.cpp` - fork() syscall
- [ ] `kernel/syscall/sys_exec.cpp` - execve() syscall
- [ ] `kernel/syscall/sys_open.cpp` - open() syscall
- [ ] `kernel/syscall/sys_close.cpp` - close() syscall
- [ ] `kernel/syscall/sys_mmap.cpp` - mmap() syscall
- [ ] `kernel/syscall/sys_brk.cpp` - brk() syscall
- [ ] `kernel/syscall/sys_wait.cpp` - waitpid() syscall
- [x] `kernel/syscall/sys_getpid.cpp` - getpid() syscall
- [ ] `kernel/syscall/sys_kill.cpp` - kill() / signal delivery syscall
- [ ] `kernel/syscall/sys_stat.cpp` - stat() / fstat() syscall
- [ ] `kernel/syscall/sys_ioctl.cpp` - ioctl() syscall
- [ ] `kernel/syscall/Makefile`

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
- [ ] `kernel/drivers/input/Makefile`

#### Network Drivers
- [ ] `kernel/drivers/net/e1000.cpp`
- [ ] `kernel/drivers/net/rtl8139.cpp`
- [ ] `kernel/drivers/net/virtio_net.cpp`
- [ ] `kernel/drivers/net/loopback.cpp` - Loopback interface
- [ ] `kernel/drivers/net/Makefile`

#### Graphics Drivers
- [ ] `kernel/drivers/graphics/vga.cpp`
- [ ] `kernel/drivers/graphics/framebuffer.cpp`
- [ ] `kernel/drivers/graphics/bochs_vbe.cpp`
- [ ] `kernel/drivers/graphics/vmware_svga.cpp`
- [ ] `kernel/drivers/graphics/virtio_gpu.cpp`
- [ ] `kernel/drivers/graphics/Makefile`

#### Sound Drivers
- [ ] `kernel/drivers/sound/sb16.cpp` - Sound Blaster 16
- [ ] `kernel/drivers/sound/ac97.cpp` - AC'97 audio
- [ ] `kernel/drivers/sound/hda.cpp` - Intel HDA
- [ ] `kernel/drivers/sound/Makefile`

### Network Stack
- [ ] `kernel/net/net.cpp` - Network core
- [ ] `kernel/net/socket.cpp` - Socket API
- [ ] `kernel/net/skbuff.cpp` - Socket buffer
- [ ] `kernel/net/eth.cpp` - Ethernet layer
- [ ] `kernel/net/arp.cpp` - ARP protocol
- [ ] `kernel/net/ip.cpp` - IPv4
- [ ] `kernel/net/tcp.cpp` - TCP
- [ ] `kernel/net/udp.cpp` - UDP
- [ ] `kernel/net/icmp.cpp` - ICMP
- [ ] `kernel/net/route.cpp` - Routing table
- [ ] `kernel/net/checksum.cpp` - Checksum utilities
- [ ] `kernel/net/Makefile`

### Library (lib)
- [ ] `kernel/lib/string.cpp` - String operations
- [ ] `kernel/lib/list.cpp` - Linked list
- [ ] `kernel/lib/rbtree.cpp` - Red-black tree
- [ ] `kernel/lib/bitmap.cpp` - Bitmap operations
- [ ] `kernel/lib/sort.cpp` - Sorting utilities
- [ ] `kernel/lib/crc32.cpp` - CRC32
- [ ] `kernel/lib/Makefile`

---

## TODO: USERLAND

### libc
- [x] `userland/libc/include/stdio.h`
- [x] `userland/libc/include/stdlib.h`
- [ ] `userland/libc/include/string.h`
- [ ] `userland/libc/include/unistd.h`
- [ ] `userland/libc/include/fcntl.h`
- [ ] `userland/libc/include/errno.h`
- [ ] `userland/libc/include/sys/types.h`
- [ ] `userland/libc/include/sys/stat.h`
- [x] `userland/libc/src/stdio/printf.c`
- [ ] `userland/libc/src/stdio/puts.c`
- [ ] `userland/libc/src/stdio/getchar.c`
- [x] `userland/libc/src/stdlib/malloc.c`
- [ ] `userland/libc/src/stdlib/free.c`
- [x] `userland/libc/src/stdlib/exit.c`
- [ ] `userland/libc/src/string/memcpy.c`
- [ ] `userland/libc/src/string/memset.c`
- [ ] `userland/libc/src/string/strlen.c`
- [ ] `userland/libc/src/string/strcmp.c`
- [ ] `userland/libc/src/string/strcpy.c`
- [ ] `userland/libc/src/unistd/read.c`
- [ ] `userland/libc/src/unistd/write.c`
- [ ] `userland/libc/src/crt/crt0.asm` - C runtime start (calls main, then exit)
- [ ] `userland/libc/Makefile`

> **NOTE FOR CURSOR**: `userland/include/syscall.h` already exists with `SYS_read=0`, `SYS_write=1`, `SYS_exit=6`. New libc wrappers must use these numbers. Do NOT redefine them in libc sources — include the existing header.

### Shell
- [x] `userland/shell/main.c` - Shell entry point (uses libc, not kernel serial directly)
- [ ] `userland/shell/parse.c` - Command parser (tokenize input)
- [ ] `userland/shell/exec.c` - Execute commands (fork+exec)
- [ ] `userland/shell/builtins.c` - Built-in commands (cd, echo, exit, help)
- [ ] `userland/shell/Makefile`

### Core Utilities
- [x] `userland/bin/init.c` - PID 1 init process (mounts fs, starts shell)
- [ ] `userland/bin/sh.c` - Minimal sh (if shell is separate from init)
- [ ] `userland/bin/ls.c`
- [ ] `userland/bin/cat.c`
- [ ] `userland/bin/echo.c`
- [ ] `userland/bin/cp.c`
- [ ] `userland/bin/mv.c`
- [ ] `userland/bin/rm.c`
- [ ] `userland/bin/mkdir.c`
- [ ] `userland/bin/pwd.c`
- [ ] `userland/bin/Makefile`

---

## TODO: DOCS
- [ ] `docs/arch/memory-map.md` - Physical and virtual memory layout
- [ ] `docs/arch/boot-sequence.md` - Full boot path (stage1 → stage2 → loader → kernel)
- [ ] `docs/arch/syscall-abi.md` - Syscall numbers and calling convention
- [ ] `docs/arch/scheduling.md` - Scheduler design and task lifecycle
- [ ] `docs/drivers/network.md`
- [ ] `docs/boot/bootloader.md`
- [ ] `docs/boot/multiboot.md`
- [ ] `docs/boot/grub-setup.md`
- [ ] `docs/porting/porting-guide.md`
- [ ] `docs/porting/new-architecture.md`
- [ ] `docs/user/installation.md`
- [ ] `docs/user/virtualbox.md`
- [ ] `docs/user/qemu.md`
- [ ] `docs/user/getting-started.md`
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
- [X] `LICENSE` - License file
- [ ] `CONTRIBUTING.md` - Contribution guidelines
- [X] `AUTHORS` - List of contributors
- [X] `.gitignore` - Git ignore rules (add build/, *.bin, *.o, *.img)
- [ ] `.gitattributes` - Git attributes
- [X] `.clang-format` - Clang format config
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
- [x] `.github/workflows/build.yml` - CI/CD build workflow
- [ ] `.github/workflows/test.yml` - Test workflow
- [ ] `.github/workflows/release.yml` - Release workflow
- [ ] `.github/ISSUE_TEMPLATE/bug_report.md`
- [ ] `.github/ISSUE_TEMPLATE/feature_request.md`
- [ ] `.github/PULL_REQUEST_TEMPLATE.md`

---

## TODO: IDE SUPPORT
- [ ] `.vscode/settings.json` - VS Code settings
- [ ] `.vscode/tasks.json` - Build tasks
- [ ] `.vscode/launch.json` - Debug configuration (attach GDB to QEMU :1234)

---

## KNOWN BUGS / INCONSISTENCIES
> None. Addressed the previously listed issues in this section.
---

## STATISTICS

**Estimated Total Files**: ~1,500-2,000
**Currently Completed**: ~248 files (~12-16%)
**Remaining**: ~1,252-1,752 files

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

> None for this section. Added x86_64 graphics module scaffolding and enabled CI build workflow.

---
