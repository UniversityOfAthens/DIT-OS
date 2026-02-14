# DIT-OS Kernel Makefile

NASM = nasm
BUILD_DIR = ../bootloader/build
KERNEL_OUT = $(BUILD_DIR)/kernel.bin

all: dirs $(KERNEL_OUT)

dirs:
	@mkdir -p $(BUILD_DIR)

$(KERNEL_OUT): kernel.asm
	$(NASM) -f bin -o $@ $<

clean:
	rm -f $(KERNEL_OUT)

.PHONY: all clean dirs
