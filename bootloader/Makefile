# DIT-OS Custom Bootloader - Simple Binary Build

NASM = nasm

SRC_DIR = src
BUILD_DIR = build
STAGE1_DIR = $(SRC_DIR)/stage1
STAGE2_DIR = $(SRC_DIR)/stage2
COMMON_DIR = $(SRC_DIR)/common
CONFIG_DIR = config
KERNEL_DIR = ../kernel

STAGE1_OUT = $(BUILD_DIR)/stage1.bin
STAGE2_OUT = $(BUILD_DIR)/stage2.bin
BOOTLOADER_OUT = $(BUILD_DIR)/bootloader.bin
DISK_IMAGE = $(BUILD_DIR)/disk.img

STAGE1_SRC = $(STAGE1_DIR)/stage1.asm
STAGE2_SRC = $(STAGE2_DIR)/stage2.asm
CONFIG_FILE = $(CONFIG_DIR)/boot.cfg
KERNEL_SRC = $(KERNEL_DIR)/kernel.asm
KERNEL_OUT = $(BUILD_DIR)/kernel.bin

NASM_BIN = -f bin -I$(SRC_DIR)/common -I$(SRC_DIR)/stage2

.PHONY: all clean run debug disk dirs

all: dirs $(DISK_IMAGE) $(STAGE1_OUT) $(STAGE2_OUT) $(BOOTLOADER_OUT) $(KERNEL_OUT)
	@echo "=========================================="
	@echo "DIT-OS Bootloader (Binary Build)"
	@echo "Stage1: $(STAGE1_OUT)"
	@echo "Stage2: $(STAGE2_OUT)"
	@echo "Disk: $(DISK_IMAGE)"
	@echo "=========================================="

$(STAGE2_OUT): $(STAGE2_SRC)
	$(NASM) $(NASM_BIN) -o $@ $<

$(STAGE1_OUT): $(STAGE1_SRC)
	$(NASM) $(NASM_BIN) -o $@ $<

$(BOOTLOADER_OUT): $(STAGE1_OUT) $(STAGE2_OUT)
	cat $^ > $@

$(DISK_IMAGE): $(BOOTLOADER_OUT) $(KERNEL_OUT)
	@dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	@dd if=$(BOOTLOADER_OUT) of=$@ conv=notrunc 2>/dev/null
	@dd if=$(KERNEL_OUT) of=$@ bs=512 seek=23 conv=notrunc 2>/dev/null

disk: dirs $(DISK_IMAGE)
	@echo "Disk image: $(DISK_IMAGE)"

run: disk
	@echo "Running DIT-OS in QEMU..."
	qemu-system-i386 -accel tcg,thread=single -drive format=raw,file=$(DISK_IMAGE),index=0,if=floppy -display none -serial mon:stdio

debug: disk
	@echo "Running DIT-OS in QEMU with GDB..."
	qemu-system-i386 -accel tcg,thread=single -drive format=raw,file=$(DISK_IMAGE),index=0,if=floppy -display none -serial mon:stdio -s -S

$(KERNEL_OUT): $(KERNEL_SRC)
	$(NASM) -f bin -o $@ $<

dirs:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
