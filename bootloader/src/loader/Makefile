# Loader: ELF load + multiboot launcher (runs at 0x10000)
# Output: flat binary for disk sector 10+

NASM = nasm
LOADER_BIN = loader.bin

.PHONY: all clean

all: $(LOADER_BIN)

$(LOADER_BIN): loader.asm
	$(NASM) -f bin -o $@ $<

clean:
	rm -f $(LOADER_BIN)
