# Stage 2 bootloader
# Default: build stage2.bin from stage2.asm (used by parent Makefile)
# Full C++ build would use i686-elf-g++ and this directory's .cpp + .asm

NASM = nasm
NASM_BIN = -f bin
COMMON = ../common

# Simple ASM-only build (current layout)
STAGE2_ASM = stage2.asm
STAGE2_BIN = stage2.bin

.PHONY: all clean asm

all: asm

asm: $(STAGE2_BIN)

# KERNEL_SECTORS comes from bootloader/Makefile, which owns the disk layout.
$(STAGE2_BIN): $(STAGE2_ASM)
	@[ -n "$(KERNEL_SECTORS)" ] || { \
		echo "ERROR: pass KERNEL_SECTORS=N (see bootloader/Makefile)" >&2; \
		exit 1; }
	$(NASM) $(NASM_BIN) -I$(COMMON) -I. -DKERNEL_SECTORS=$(KERNEL_SECTORS) \
		-o $(STAGE2_BIN) $(STAGE2_ASM)

clean:
	rm -f $(STAGE2_BIN) *.o
