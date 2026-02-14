# DIT-OS

DIT-OS is a scalable, modular and secure operating system designed for modern computing environments. Download now for free using the link 


## Requirements

To build DIT-OS, you need the following tools installed on your system:

### Required Tools

| Tool | Purpose | Installation |
|------|---------|--------------|
| **NASM** | x86/x86-64 assembler | `sudo apt install nasm` (Debian/Ubuntu)<br>`sudo pacman -S nasm` (Arch)<br>`brew install nasm` (macOS) |
| **GCC** | C/C++ compiler | `sudo apt install build-essential` (Debian/Ubuntu)<br>`sudo pacman -S gcc` (Arch)<br>Xcode Command Line Tools (macOS) |
| **QEMU** | x86 emulator for testing | `sudo apt install qemu-system-x86` (Debian/Ubuntu)<br>`sudo pacman -S qemu-full` (Arch)<br>`brew install qemu` (macOS) |
| **GNU Make** | Build automation | Usually pre-installed, or:<br>`sudo apt install make` (Debian/Ubuntu)<br>`sudo pacman -S make` (Arch) |
| **GRUB** | Bootloader utilities | `sudo apt install grub-common grub-pc-bin` (Debian/Ubuntu)<br>`sudo pacman -S grub` (Arch) |
| **xorriso** | ISO image creation | `sudo apt install xorriso` (Debian/Ubuntu)<br>`sudo pacman -S libisoburn` (Arch)<br>`brew install xorriso` (macOS) |

### Quick Install (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install nasm gcc make qemu-system-x86 grub-common grub-pc-bin xorriso
```

### Quick Install (Arch Linux)
```bash
sudo pacman -S nasm gcc make qemu-full grub libisoburn
```

### Quick Install (macOS)
```bash
brew install nasm qemu xorriso
# GCC and Make come with Xcode Command Line Tools
xcode-select --install
```

## Compilation

To compile DIT-OS, follow these steps:
1. Clone the repository:
   ```bash
   git clone https://github.com/UniversityOfAthens/DIT-OS.git
   ```

2. Navigate to the project directory:
   ```bash
   cd DIT-OS
   ```

3. Build the project using the provided Makefile:
   ```bash
    make
   ```


## Developers

So far, the following developers have contributed to this project:

<a href="https://github.com/UniversityOfAthens/DIT-OS/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=UniversityOfAthens/DIT-OS" />
</a>

## License

This project is licensed under the OpenGPL-v3 License - see the [LICENSE](LICENSE) file for details.