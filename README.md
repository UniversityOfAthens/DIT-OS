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

With Nix you can use the flake instead: `nix develop` opens a shell with
everything above plus `gdb`, `clang-tools` and `xxd`.

## Build and run

DIT-OS boots with its own bootloader (no GRUB). The kernel and userland are
built with the host `gcc`/`g++` in 32-bit mode (`-m32`); no cross-compiler is
needed yet.

### 1. Build

From the project root:

```bash
rm -rf bootloader/build kernel/build   # clean build (recommended)
make -C kernel                         # kernel ELF -> bootloader/build/kernel.bin
make -C bootloader all                 # stage1, stage2, loader, disk image
```

`make -C bootloader all` also rebuilds the kernel, so on its own it is enough
for everyday work. `./scripts/build.sh` runs both steps.

Build outputs in `bootloader/build/`:

| File | What it is |
|------|------------|
| `stage1.bin` | 512-byte MBR |
| `stage2.bin` | Boot menu, loads the loader and kernel |
| `loader.bin` | Enters protected mode, loads the kernel ELF, jumps to it |
| `kernel.bin` | Kernel ELF with symbols (use this with GDB) |
| `kernel.img` | Stripped copy of `kernel.bin` that is written to the disk |
| `disk.img` | 1.44 MB floppy image |

Disk layout: stage1 at sector 0, stage2 at sectors 2–10, loader at 11–36,
kernel from sector 37. Stage2 reads a fixed number of kernel sectors
(`KERNEL_SECTORS` in `bootloader/Makefile`, currently 200 = 100 KB); the build
fails if `kernel.img` is bigger.

### 2. Run in QEMU

```bash
make -C bootloader run
```

or `./scripts/run-qemu.sh`, which builds first if needed and boots a snapshot
so the disk image is never modified.

QEMU runs without a window (`-display none`); everything appears in your
terminal through the serial port:

1. Stage1 and stage2 messages, then the boot menu (**Boot DIT-OS**,
   **Recovery Mode**, **Reboot**). Use the arrow keys or `1`/`2`/`3`, then
   Enter.
2. The loader prints `LOEK` (loader started, protected mode, ELF found,
   jumping to kernel).
3. Kernel boot log: CPU setup (GDT, IDT, TSS, PIC), drivers (serial, keyboard,
   mouse, timer, RTC), scheduler and ramfs.
4. The shell prompt, `user@dit:~$`.

Shell commands:

| Command | What it does |
|---------|--------------|
| `ls [path]` | List a directory (default `/`) |
| `cat <file>` | Print a file, e.g. `cat hello.txt` |
| `echo <text>` | Print text |
| `export NAME=value` | Set `USER`, `HOST`, `PWD` or `PS1` (prompt) |
| `exit` | Power off QEMU |

To quit QEMU at any point, press **Ctrl+A** then **X**.

If the kernel hits a CPU exception, it prints a `KERNEL PANIC` report with the
exception name, error code and `EIP` to the terminal, then halts.

### 3. Debug (QEMU + GDB)

```bash
make -C bootloader debug
```

QEMU starts paused and waits for GDB on port 1234. In another terminal:

```bash
gdb -ex "target remote :1234" \
    -ex "symbol-file bootloader/build/kernel.bin" \
    -ex "break kernel_main" \
    -ex "continue"
```

The boot menu still needs Enter in the QEMU terminal before `kernel_main` is
reached.

## Build options

- **Kernel only**: `make -C kernel` (output: `bootloader/build/kernel.bin`)
- **Disk image only**: `make -C bootloader disk`
- **Clean**: `make -C kernel clean` and `make -C bootloader clean`

## Alternative: CMake (kernel only, experimental)

```bash
mkdir build && cd build
cmake ..
make kernel
```

The CMake build does not yet compile the userland shell, so the Makefiles are
the supported path.

## Project layout (brief)

- **bootloader/** – Stage1 (512-byte MBR), Stage2 (menu, load kernel), scripts, headers. Builds `build/disk.img`.
- **kernel/** – 32-bit x86 kernel (the `arch/x86_64` directory name is historical): CPU setup, drivers, scheduler, syscalls, VFS and ramfs. Builds ELF to `bootloader/build/kernel.bin`.
- **userland/** – Shell, init and a small libc. Currently linked into the kernel image and run as the init task.
- **PROJECT_STRUCTURE.md** – Full layout. **TODO.md** – Task list. **ISSUES.md** – Known bugs. **CHANGELOG.md** – Release history.

## Developers

<a href="https://github.com/UniversityOfAthens/DIT-OS/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=UniversityOfAthens/DIT-OS" />
</a>

## License

This project is licensed under the OpenGPL-v3 License – see the [LICENSE](LICENSE) file for details.
