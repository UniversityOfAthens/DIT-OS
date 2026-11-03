# DIT-OS Custom Bootloader - Full boot path (ELF kernel via loader)

NASM = nasm
# Same toolchain choice as kernel/Makefile (host objcopy handles elf32 too)
CROSS_COMPILE ?= $(if $(shell command -v i686-elf-objcopy 2>/dev/null),i686-elf-,)
OBJCOPY = $(CROSS_COMPILE)objcopy

SRC_DIR = src
BUILD_DIR = build
STAGE1_DIR = $(SRC_DIR)/stage1
STAGE2_DIR = $(SRC_DIR)/stage2
LOADER_DIR = $(SRC_DIR)/loader
COMMON_DIR = $(SRC_DIR)/common
KERNEL_DIR = ../kernel

STAGE1_OUT = $(BUILD_DIR)/stage1.bin
STAGE2_OUT = $(BUILD_DIR)/stage2.bin
LOADER_OUT = $(BUILD_DIR)/loader.bin
KERNEL_OUT = $(BUILD_DIR)/kernel.bin
# Stripped copy of kernel.bin written to disk (kernel.bin keeps symbols for GDB)
KERNEL_IMG = $(BUILD_DIR)/kernel.img
DISK_IMAGE = $(BUILD_DIR)/disk.img

# Disk layout: 0=stage1, 2-10=stage2 (9 sectors), 11-36=loader, 37+=kernel
DISK_SECTOR_STAGE2  = 2
DISK_SECTOR_LOADER  = 11
DISK_SECTOR_KERNEL  = 37
LOADER_SECTORS      = 26
# Sectors stage2 reads for the kernel; passed to stage2.asm and checked below
KERNEL_SECTORS      = 200
SECTOR_SIZE         = 512

STAGE1_SRC = $(STAGE1_DIR)/stage1.asm
STAGE2_SRC = $(STAGE2_DIR)/stage2.asm
NASM_BIN = -f bin -I$(SRC_DIR)/common -I$(STAGE2_DIR)

.PHONY: all clean run debug disk dirs kernel loader FORCE

all: dirs $(DISK_IMAGE)
	@echo "=========================================="
	@echo "DIT-OS Bootloader (full boot path)"
	@echo "Stage1: $(STAGE1_OUT)"
	@echo "Stage2: $(STAGE2_OUT)"
	@echo "Loader: $(LOADER_OUT)"
	@echo "Kernel: $(KERNEL_OUT) (ELF)"
	@echo "Disk:   $(DISK_IMAGE)"
	@echo "=========================================="

$(STAGE1_OUT): $(STAGE1_SRC)
	$(NASM) $(NASM_BIN) -o $@ $<

$(STAGE2_OUT): $(STAGE2_SRC) Makefile
	$(NASM) $(NASM_BIN) -DKERNEL_SECTORS=$(KERNEL_SECTORS) \
		-DLOADER_SECTORS=$(LOADER_SECTORS) -o $@ $<

$(LOADER_OUT): dirs
	$(MAKE) -C $(LOADER_DIR) all
	cp $(LOADER_DIR)/loader.bin $@
	@size=$$(wc -c < $@); max=$$(( $(LOADER_SECTORS) * $(SECTOR_SIZE) )); \
	if [ "$$size" -gt "$$max" ]; then \
		echo "ERROR: loader is $$size bytes, stage2 loads only $$max" \
			"(LOADER_SECTORS=$(LOADER_SECTORS))" >&2; \
		rm -f $@; exit 1; \
	fi

# Kernel ELF from kernel/Makefile
$(KERNEL_OUT): FORCE
	$(MAKE) -C $(KERNEL_DIR) all

FORCE:

# Stage2 loads exactly KERNEL_SECTORS; a bigger image would boot truncated.
$(KERNEL_IMG): $(KERNEL_OUT)
	$(OBJCOPY) --strip-all $< $@
	@size=$$(wc -c < $@); max=$$(( $(KERNEL_SECTORS) * $(SECTOR_SIZE) )); \
	if [ "$$size" -gt "$$max" ]; then \
		echo "ERROR: kernel image is $$size bytes, stage2 loads only" \
			"$$max (KERNEL_SECTORS=$(KERNEL_SECTORS))" >&2; \
		rm -f $@; exit 1; \
	fi

$(DISK_IMAGE): $(STAGE1_OUT) $(STAGE2_OUT) $(LOADER_OUT) $(KERNEL_IMG)
	@dd if=/dev/zero of=$@ bs=512 count=2880 status=none
	@dd if=$(STAGE1_OUT) of=$@ conv=notrunc status=none
	@dd if=$(STAGE2_OUT) of=$@ bs=512 seek=$(DISK_SECTOR_STAGE2) conv=notrunc status=none
	@dd if=$(LOADER_OUT) of=$@ bs=512 seek=$(DISK_SECTOR_LOADER) conv=notrunc status=none
	@dd if=$(KERNEL_IMG) of=$@ bs=512 seek=$(DISK_SECTOR_KERNEL) conv=notrunc status=none
	@echo "Disk image: $@"

disk: dirs $(DISK_IMAGE)

run: disk
	@echo "Running DIT-OS in QEMU..."
	qemu-system-i386 -accel tcg,thread=single -drive format=raw,file=$(DISK_IMAGE),index=0,if=floppy -display none -serial mon:stdio -no-reboot -m 32M

debug: disk
	@echo "Running DIT-OS in QEMU with GDB..."
	qemu-system-i386 -accel tcg,thread=single -drive format=raw,file=$(DISK_IMAGE),index=0,if=floppy -display none -serial mon:stdio -no-reboot -no-shutdown -m 32M -s -S

dirs:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(LOADER_DIR) clean 2>/dev/null || true
