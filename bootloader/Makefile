# DIT-OS Custom Bootloader - Phase 1

# Configuration
NASM = nasm
LD = ld

# Directories
SRC_DIR = src
BUILD_DIR = build
STAGE1_DIR = $(SRC_DIR)/stage1
STAGE2_DIR = $(SRC_DIR)/stage2
COMMON_DIR = $(SRC_DIR)/common

# Output files
STAGE1_OUT = $(BUILD_DIR)/stage1.bin
STAGE2_OUT = $(BUILD_DIR)/stage2.bin
BOOTLOADER_OUT = $(BUILD_DIR)/bootloader.bin

# Stage1 source
STAGE1_SRC = $(STAGE1_DIR)/stage1.asm

# Stage2 ASM sources (now all in stage2.asm - other files optional)
STAGE2_SRC = $(STAGE2_DIR)/stage2.asm

# NASM flags
NASM_FLAGS = -f elf32 -g -I$(SRC_DIR)/common -I$(SRC_DIR)/stage2
NASM_BIN_FLAGS = -f bin -I$(SRC_DIR)/common -I$(SRC_DIR)/stage2

# LD flags
LD_FLAGS = -m elf_i386 --oformat binary -e _start

# Default target
.PHONY: all clean run debug
all: dirs $(STAGE1_OUT) $(STAGE2_OUT) $(BOOTLOADER_OUT)
	@echo "Bootloader build complete!"
	@echo "Stage1: $(STAGE1_OUT)"
	@echo "Stage2: $(STAGE2_OUT)"
	@echo "Combined: $(BOOTLOADER_OUT)"

# Run in QEMU (no GUI)
run: $(BOOTLOADER_OUT)
	@echo "Running bootloader in QEMU..."
	@echo "(SeaBIOS messages below are from QEMU firmware, not the bootloader)"
	qemu-system-i386 -drive format=raw,file=$(BOOTLOADER_OUT),index=0,if=floppy -nographic

# Debug mode with GDB
debug: $(BOOTLOADER_OUT)
	@echo "Running bootloader in QEMU with GDB..."
	qemu-system-i386 -drive format=raw,file=$(BOOTLOADER_OUT),index=0,if=floppy -nographic -s -S

dirs:
	@mkdir -p $(BUILD_DIR)

# Build Stage1 (binary format for boot sector)
$(STAGE1_OUT): $(STAGE1_SRC) $(COMMON_DIR)/constants.inc
	$(NASM) $(NASM_BIN_FLAGS) -o $@ $<

# Build Stage2 (use bin format for flat binary)
$(STAGE2_OUT): $(STAGE2_SRC) $(COMMON_DIR)/macros.inc $(COMMON_DIR)/constants.inc
	$(NASM) $(NASM_BIN_FLAGS) -o $@ $<

# Combine Stage1 and Stage2 into final bootloader
$(BOOTLOADER_OUT): $(STAGE1_OUT) $(STAGE2_OUT)
	cat $^ > $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
