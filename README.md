# DIT-OS

DIT-OS is a modular operating system with a **custom bootloader** (stage1 + stage2) and an x86 kernel. It boots from a floppy-style disk image in QEMU.

## Requirements

| Tool   | Purpose              | Install (Debian/Ubuntu)        |
|--------|----------------------|--------------------------------|
| **NASM** | Assembler           | `sudo apt install nasm`       |
| **GCC**  | C/C++ compiler      | `sudo apt install build-essential` |
| **GNU Make** | Build             | `sudo apt install make`       |
| **QEMU** | Emulator (run OS)   | `sudo apt install qemu-system-x86` |

Quick install (Debian/Ubuntu):

```bash
sudo apt install nasm build-essential make qemu-system-x86
```

## Build and run (current way)

We use the **custom bootloader** (no GRUB). Build the bootloader and disk image, then run in QEMU.

### 1. Build everything (recommended)

From the project root:

```bash
# Build kernel (ELF → bootloader/build/kernel.bin)
make -C kernel

# Build bootloader + disk image (stage1 + stage2 + kernel on floppy image)
make -C bootloader all
```

Or use the bootloader’s script (if present):

```bash
./bootloader/scripts/build.sh all
```

### 2. Run in QEMU

From the project root:

```bash
make -C bootloader run
```

Or from the bootloader directory:

```bash
cd bootloader && make run
```

This starts QEMU with the floppy disk image (`bootloader/build/disk.img`) and no graphical window (`-nographic`). You should see the stage1 message, then the stage2 menu (e.g. “Boot DIT-OS”, “Recovery Mode”, “Reboot”). Choose **Boot DIT-OS** to load the kernel.

### What “Boot DIT-OS” does today

The disk image is built with the **minimal kernel** (`kernel/kernel.asm`): a small binary loaded at 0x10000. When you select **Boot DIT-OS**:

1. Stage2 loads 64 sectors from the disk (starting at sector 23) into memory at **0x10000** and jumps to it.
2. The minimal kernel runs in 16-bit, then sets up a GDT, enables the A20 line, and switches to **protected mode**.
3. It prints a short message (e.g. “DIT-OS Kernel loaded!” and “PM!”) to the VGA text buffer, then **halts**.

So you do get into protected mode with basic CPU setup (GDT). The **full kernel** (ELF with multiboot, `init.cpp`, `main.cpp`, IDT, etc.) is built by `make -C kernel` but is **not** currently put on the disk or started by the bootloader. That kernel is intended to run at **0x100000** with multiboot magic in EAX/EBX; wiring that in would require stage2 to load the ELF and jump with that convention. Full CPU management (IDT, interrupts, etc.) lives in that full kernel and is not used until that path is implemented.

### 3. Debug (QEMU + GDB)

```bash
make -C bootloader debug
```

Then in another terminal, attach GDB to QEMU (e.g. `target remote :1234`).

## Build options

- **Kernel only**  
  `make -C kernel`  
  Output: `bootloader/build/kernel.bin`

- **Bootloader disk only** (uses existing `kernel.bin`; if missing, the bootloader Makefile builds a minimal `kernel/kernel.asm` stub)  
  `make -C bootloader disk`

- **Clean**  
  `make -C kernel clean`  
  `make -C bootloader clean`

## Alternative: CMake (kernel only)

You can build the kernel with CMake instead of the kernel Makefile:

```bash
mkdir build && cd build
cmake ..
make kernel
```

The kernel binary is still written to `bootloader/build/kernel.bin`. After that, build the disk and run as above:

```bash
make -C bootloader all
make -C bootloader run
```

## Project layout (brief)

- **bootloader/** – Stage1 (512-byte MBR), Stage2 (menu, load kernel), scripts, headers. Builds `build/disk.img`.
- **kernel/** – x86_64 kernel (boot, CPU, multiboot, init, main). Builds ELF to `bootloader/build/kernel.bin`.
- **PROJECT_STRUCTURE.md** – Full layout. **TODO.md** – Task list (custom bootloader is primary; GRUB optional).

## Developers

<a href="https://github.com/UniversityOfAthens/DIT-OS/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=UniversityOfAthens/DIT-OS" />
</a>

## License

This project is licensed under the OpenGPL-v3 License – see the [LICENSE](LICENSE) file for details.
