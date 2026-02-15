# DIT-OS Custom Bootloader - Modern Build System

NASM = nasm
LD = ld

SRC_DIR = src
BUILD_DIR = build
STAGE1_DIR = $(SRC_DIR)/stage1
STAGE2_DIR = $(SRC_DIR)/stage2
COMMON_DIR = $(SRC_DIR)/common
CONFIG_DIR = config
KERNEL_DIR = ../kernel

STAGE1_OUT = $(BUILD_DIR)/stage1.bin
STAGE2_ELF = $(BUILD_DIR)/stage2.elf
STAGE2_OUT = $(BUILD_DIR)/stage2.bin
BOOTLOADER_OUT = $(BUILD_DIR)/bootloader.bin
DISK_IMAGE = $(BUILD_DIR)/disk.img

STAGE1_SRC = $(STAGE1_DIR)/stage1.asm
CONFIG_FILE = $(CONFIG_DIR)/boot.cfg
KERNEL_SRC = $(KERNEL_DIR)/kernel.asm
KERNEL_OUT = $(BUILD_DIR)/kernel.bin

# Assembly flags
NASM_BIN = -f bin -I$(SRC_DIR)/common -I$(SRC_DIR)/stage2
NASM_ELF = -f elf32 -I$(SRC_DIR)/common -I$(SRC_DIR)/stage2

# Object files for ELF build
OBJS = $(BUILD_DIR)/stage2_main.o $(BUILD_DIR)/stage2_core.o

.PHONY: all clean run debug disk dirs elf binary

# Default: build ELF version
all: elf

# ELF build (modular, all features)
elf: dirs $(STAGE1_OUT) $(STAGE2_OUT) $(BOOTLOADER_OUT) $(KERNEL_OUT)
	@echo "=========================================="
	@echo "DIT-OS Bootloader (ELF Build)"
	@echo "Stage1: $(STAGE1_OUT)"
	@echo "Stage2: $(STAGE2_OUT)"
	@echo "=========================================="

# Binary build (legacy, simpler)
binary: dirs $(BUILD_DIR)/stage2_legacy.bin $(BOOTLOADER_OUT) $(KERNEL_OUT)
	@echo "=========================================="
	@echo "DIT-OS Bootloader (Binary Build)"
	@echo "=========================================="

$(STAGE2_OUT): $(OBJS) $(STAGE2_DIR)/stage2.ld
	@echo "Linking ELF modules..."
	$(LD) -m elf_i386 -T $(STAGE2_DIR)/stage2.ld -o $(STAGE2_ELF) $(OBJS)
	@echo "Extracting binary..."
	$(LD) -m elf_i386 --oformat binary -o $(STAGE2_OUT) -e _start $(STAGE2_ELF)

$(BUILD_DIR)/stage2_main.o: $(STAGE2_DIR)/stage2_main.asm
	$(NASM) $(NASM_ELF) -o $@ $<

$(BUILD_DIR)/stage2_core.o: $(STAGE2_DIR)/stage2_core.asm
	$(NASM) $(NASM_ELF) -o $@ $<

$(STAGE1_OUT): $(STAGE1_SRC) $(COMMON_DIR)/constants.inc
	$(NASM) $(NASM_BIN) -o $@ $<

$(BOOTLOADER_OUT): $(STAGE1_OUT) $(STAGE2_OUT)
	cat $^ > $@

$(DISK_IMAGE): $(BOOTLOADER_OUT) $(CONFIG_FILE) $(KERNEL_OUT)
	@dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	@dd if=$(BOOTLOADER_OUT) of=$@ bs=512 conv=notrunc 2>/dev/null
	@dd if=$(CONFIG_FILE) of=$@ bs=512 seek=19 conv=notrunc 2>/dev/null
	@dd if=$(KERNEL_OUT) of=$@ bs=512 seek=23 conv=notrunc 2>/dev/null

disk: dirs $(DISK_IMAGE)
	@echo "Disk image: $(DISK_IMAGE)"

run: disk
	@echo "Running DIT-OS in QEMU..."
	qemu-system-i386 -drive format=raw,file=$(DISK_IMAGE),index=0,if=floppy -nographic

debug: disk
	@echo "Running DIT-OS in QEMU with GDB..."
	qemu-system-i386 -drive format=raw,file=$(DISK_IMAGE),index=0,if=floppy -nographic -s -S

$(KERNEL_OUT): $(KERNEL_SRC)
	$(NASM) -f bin -o $@ $<

dirs:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
