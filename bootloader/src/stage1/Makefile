# Stage 1 bootloader (512 bytes)
NASM = nasm
SRC = stage1.asm
OUT = stage1.bin
COMMON = ../common

.PHONY: all clean

all: $(OUT)

$(OUT): $(SRC)
	$(NASM) -f bin -I$(COMMON) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)
