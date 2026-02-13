# Future Plans: Custom Bootloader Implementation

**Status**: Currently using GRUB bootloader
**Goal**: Replace with fully custom bootloader implementation
**Timeline**: Long-term project (months to years)
**Estimated Effort**: 5,000-10,000+ lines of code

---

## Overview

This document outlines the roadmap for replacing the GRUB bootloader with a fully custom, feature-rich bootloader that maintains the same theme and configuration capabilities.

---

## Current State

**Using GRUB** (`boot/grub/`):
- Working bootloader with minimal effort
- Theme support with `theme.txt`
- Configuration via `grub.cfg`
- Font rendering with PF2 format
- Background images
- Menu system
- Multiboot protocol support

**Files to Eventually Replace**:
- `boot/grub/grub.cfg` → `bootloader/stage2/config.cpp`
- `boot/grub/themes/dit/theme.txt` → Custom theme parser
- `boot/grub/themes/dit/fonts/*.pf2` → Custom font loader/renderer
- `boot/grub/themes/dit/assets/delis.png` → Custom image loader

---

## Phase 1: Basic Boot (Foundation)

**Estimated Time**: 2-4 weeks
**Lines of Code**: ~500-1,000

### Stage 1 Bootloader (boot.asm)
- 512-byte boot sector loaded at 0x7C00
- MBR partition table support
- Load Stage 2 from disk
- Basic error messages (text mode)
- **Location**: `bootloader/src/stage1/boot.asm`

### Stage 2 Bootloader (Basic)
- Real-mode initialization
- Enable A20 line
- Load GDT (Global Descriptor Table)
- Switch to protected mode
- Basic disk I/O (read kernel from disk)
- Text-mode menu (no graphics yet)
- **Location**: `bootloader/src/stage2/stage2.asm`, `stage2.cpp`

### Deliverable
- Custom bootloader that can load and jump to kernel
- Simple text menu: "1. Boot DIT-OS  2. Advanced options"
- No themes, no config files yet

---

## Phase 2: Configuration System

**Estimated Time**: 3-6 weeks
**Lines of Code**: ~1,000-2,000

### Configuration Parser
- Custom config file format (similar to GRUB's)
- Parse boot entries
- Parse kernel parameters
- Parse timeout settings
- **Location**: `bootloader/src/stage2/config.cpp`

### Config File Format
```
# Custom bootloader config
TIMEOUT=5
DEFAULT=0

ENTRY "DIT OS" {
    KERNEL /boot/kernel.bin
    CMDLINE "root=/dev/sda1 quiet"
}

ENTRY "DIT OS (Debug Mode)" {
    KERNEL /boot/kernel.bin
    CMDLINE "root=/dev/sda1 debug loglevel=7"
}

ENTRY "Recovery Mode" {
    KERNEL /boot/recovery.bin
}
```

### Menu System
- Text-mode menu with keyboard navigation
- Arrow keys to select
- Enter to boot
- Highlight selected entry
- Countdown timer
- **Location**: `bootloader/src/stage2/menu.cpp`

### Deliverable
- Config file support
- Multiple boot entries
- Keyboard navigation
- Still text-mode only

---

## Phase 3: Graphics Foundation

**Estimated Time**: 6-10 weeks
**Lines of Code**: ~2,000-3,000

### VESA/VGA Mode Setup
- Detect VESA BIOS Extensions (VBE)
- Set graphics mode (800x600, 1024x768, etc.)
- Linear framebuffer access
- Mode switching
- **Location**: `bootloader/src/stage2/video.asm`, `video.cpp`

### Graphics Primitives
- Pixel plotting
- Line drawing (Bresenham's algorithm)
- Rectangle drawing
- Circle drawing (optional)
- Fill operations
- **Location**: `bootloader/src/stage2/gfx/primitives.cpp`

### Image Loading
- PNG decoder (compression, filtering)
- JPEG decoder (optional, complex)
- BMP decoder (simple, good for testing)
- Image scaling/resizing
- **Location**: `bootloader/src/stage2/gfx/image.cpp`

### Deliverable
- Graphics mode menu
- Can display background images
- Basic shapes and rendering

---

## Phase 4: Font Rendering System

**Estimated Time**: 4-6 weeks
**Lines of Code**: ~1,000-1,500

### Font Format Support
- TTF/OTF font parser (extremely complex, maybe skip)
- PSF (PC Screen Font) support (simpler)
- Custom bitmap font format
- BDF font support
- **Location**: `bootloader/src/stage2/gfx/font.cpp`

### Font Rendering
- Glyph rasterization
- Anti-aliasing (optional)
- Font scaling
- Bold/italic simulation
- Unicode support (basic BMP)
- **Location**: `bootloader/src/stage2/gfx/text.cpp`

### Deliverable
- Display text in graphics mode
- Multiple font support
- Proper text positioning

---

## Phase 5: Theme Engine

**Estimated Time**: 4-8 weeks
**Lines of Code**: ~1,500-2,500

### Theme Parser
- Custom theme file format
- Parse colors, fonts, images
- Parse component positions
- Parse animations (optional)
- **Location**: `bootloader/src/stage2/theme/parser.cpp`

### Theme File Format
```
# DIT Bootloader Theme

BACKGROUND {
    image = "/boot/themes/dit/background.png"
    color = "#000000"
}

TITLE {
    text = "DIT OS"
    font = "/boot/themes/dit/fonts/title.ttf"
    size = 48
    color = "#FFFFFF"
    x = 50%
    y = 10%
}

MENU {
    x = 50%
    y = 50%
    width = 80%
    height = 60%
    
    item_font = "/boot/themes/dit/fonts/menu.ttf"
    item_size = 24
    item_color = "#CCCCCC"
    item_selected_color = "#FFFFFF"
    item_selected_bg = "#3366CC"
    
    timeout_bar = true
    timeout_color = "#00FF00"
}
```

### Theme Components
- Background rendering
- Title text
- Boot menu widget
- Progress bars
- Clock widget (optional)
- Help text
- **Location**: `bootloader/src/stage2/theme/components.cpp`

### Deliverable
- Full theme support
- Similar capabilities to current GRUB theme
- Customizable colors, fonts, images

---

## Phase 6: Advanced Features

**Estimated Time**: 6-12 weeks
**Lines of Code**: ~2,000-3,000

### Mouse Support
- PS/2 mouse driver
- USB mouse driver (HID)
- Mouse cursor rendering
- Click detection on menu items
- **Location**: `bootloader/src/stage2/input/mouse.cpp`

### Multiboot Support
- Multiboot header parsing
- Multiboot2 support
- Module loading
- Memory map passing to kernel
- **Location**: `bootloader/src/stage2/multiboot.cpp`

### UEFI Support
- UEFI boot services
- GOP (Graphics Output Protocol)
- UEFI file system access
- Exit boot services before jumping to kernel
- **Location**: `bootloader/src/stage2/uefi/` (separate build)

### Disk & Filesystem
- EXT2 filesystem reader
- FAT32 filesystem reader
- ISO9660 support (for CD/DVD)
- NVMe driver
- AHCI/SATA driver
- **Location**: `bootloader/src/stage2/fs/`, `drivers/`

### Additional Features
- Boot splash screen
- Boot log display
- Emergency shell
- Boot from network (PXE)
- Encrypted volume support (LUKS)
- **Location**: Various

---

## Phase 7: Testing & Optimization

**Estimated Time**: 4-8 weeks

### Testing
- Test on real hardware (multiple PCs)
- Test in VMs (QEMU, VirtualBox, VMware)
- Test with different graphics cards
- Test with different storage devices
- Memory leak testing
- Edge case handling

### Optimization
- Boot speed optimization
- Memory usage optimization
- Graphics rendering optimization
- Disk read optimization
- Code size optimization

### Documentation
- Bootloader architecture docs
- Theme development guide
- Configuration reference
- Porting guide

---

## Migration Strategy

### Step-by-Step Replacement

1. **Keep GRUB during development**
   - Use GRUB for daily kernel development
   - Develop custom bootloader in parallel
   - Test custom bootloader in VM first

2. **Dual-boot setup**
   - Install custom bootloader alongside GRUB
   - Chainload between them for testing
   - Verify custom bootloader works perfectly

3. **Gradual feature parity**
   - Implement features one by one
   - Compare behavior with GRUB
   - Fix discrepancies

4. **Final switch**
   - Remove GRUB from ISO
   - Custom bootloader becomes default
   - Keep GRUB as fallback option initially

---

## File Structure (Future)

```
bootloader/
├── src/
│   ├── stage1/
│   │   ├── boot.asm              # 512-byte boot sector
│   │   └── Makefile
│   ├── stage2/
│   │   ├── stage2.asm            # Entry point
│   │   ├── stage2.cpp            # C wrapper
│   │   ├── protected_mode.asm    # Mode switching
│   │   ├── long_mode.asm         # 64-bit mode
│   │   ├── a20.asm               # A20 line
│   │   ├── disk.cpp              # Disk I/O
│   │   ├── fs.cpp                # Filesystem interface
│   │   ├── fat.cpp               # FAT support
│   │   ├── ext2.cpp              # EXT2 support
│   │   ├── memory.cpp            # Memory detection
│   │   ├── video.cpp             # Video setup
│   │   ├── config.cpp            # Config parser
│   │   ├── menu.cpp              # Menu system
│   │   ├── multiboot.cpp         # Multiboot support
│   │   ├── gfx/                  # Graphics subsystem
│   │   │   ├── primitives.cpp    # Drawing primitives
│   │   │   ├── image.cpp         # Image loading
│   │   │   ├── font.cpp          # Font handling
│   │   │   └── text.cpp          # Text rendering
│   │   ├── theme/                # Theme engine
│   │   │   ├── parser.cpp        # Theme file parser
│   │   │   ├── components.cpp    # UI components
│   │   │   └── renderer.cpp      # Theme rendering
│   │   ├── input/                # Input handling
│   │   │   ├── keyboard.cpp      # Keyboard driver
│   │   │   └── mouse.cpp         # Mouse driver
│   │   └── Makefile
│   └── uefi/                     # UEFI support (future)
├── include/
│   ├── boot.hpp
│   ├── config.hpp
│   ├── disk.hpp
│   ├── fs.hpp
│   ├── gfx.hpp
│   ├── theme.hpp
│   └── types.hpp
├── themes/
│   └── dit/                      # Ported from GRUB theme
│       ├── theme.cfg             # Custom format
│       ├── background.png
│       └── fonts/
│           ├── regular.ttf
│           └── bold.ttf
├── scripts/
│   ├── build.sh
│   └── install.sh
└── build/
    ├── stage1.bin
    └── stage2.bin
```

---

## Reference Resources

### Specifications
- [BIOS Interrupts](http://www.ctyme.com/intr/rb-0000.htm)
- [VESA BIOS Extensions](https://wiki.osdev.org/VESA_Video_Extensions)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/)
- [Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/)
- [UEFI Specification](https://uefi.org/specifications)

### Learning Resources
- [OSDev Wiki - Bootloader](https://wiki.osdev.org/Bootloader)
- [Writing a Bootloader](https://wiki.osdev.org/Bootloader_Tutorial)
- [Rolling Your Own Bootloader](https://wiki.osdev.org/Rolling_Your_Own_Bootloader)
- [Limine Bootloader Source](https://github.com/limine-bootloader/limine) - Reference implementation

### Fonts & Graphics
- [FreeType Library](https://freetype.org/) - Reference for font rendering
- [PNG Specification](http://www.libpng.org/pub/png/spec/)
- [TinyPNG Decoder](https://github.com/lvandeve/lodepng) - Simple PNG decoder reference

---

## Summary

| Phase | Time | LOC | Deliverable |
|-------|------|-----|-------------|
| Phase 1: Basic Boot | 2-4 weeks | 500-1,000 | Working bootloader (text mode) |
| Phase 2: Configuration | 3-6 weeks | 1,000-2,000 | Config files, text menu |
| Phase 3: Graphics | 6-10 weeks | 2,000-3,000 | Graphics mode, images |
| Phase 4: Fonts | 4-6 weeks | 1,000-1,500 | Font rendering |
| Phase 5: Themes | 4-8 weeks | 1,500-2,500 | Full theme engine |
| Phase 6: Advanced | 6-12 weeks | 2,000-3,000 | Mouse, UEFI, Multiboot |
| Phase 7: Polish | 4-8 weeks | - | Testing, optimization |
| **TOTAL** | **~6-12 months** | **~8,000-13,000+** | **Feature-complete bootloader** |

---

## Notes

- This is a **massive undertaking** - comparable to writing the kernel itself
- Consider using **Limine** as an intermediate step - it supports themes but is simpler than GRUB
- Writing a custom bootloader is **primarily an educational exercise**
- GRUB/Limine are battle-tested and widely used for a reason
- Only proceed if you genuinely want to learn low-level boot details

**Alternative Path**: Use Limine instead of custom
- Limine supports themes and config files
- Much simpler than writing from scratch
- Still boots your kernel
- You can focus on kernel development instead

---

*Created: 2026-02-14*
*Status: Planning Phase*
