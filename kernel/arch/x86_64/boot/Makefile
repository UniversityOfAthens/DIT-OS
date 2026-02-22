# DIT-OS Kernel - Boot components (x86_64)
# Build from kernel/ with: make (boot objects are built by top-level Makefile)
# Or build trampoline binary from here: make trampoline.bin

NASM = nasm
CXX = g++
ARCH = x86_64

BOOT_DIR = .
NASM_FLAGS = -f elf32 -g
CXX_FLAGS = -std=c++17 -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -m32
CXX_FLAGS += -I../../../../include -I../../../include

ASM_SOURCES = multiboot.asm multiboot2.asm entry.asm
CPP_SOURCES = init.cpp main.cpp

ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

.PHONY: all clean trampoline.bin

all: $(ASM_OBJECTS) $(CPP_OBJECTS)

# Trampoline is a flat binary for copying to 0x1000 (built separately)
trampoline.bin: trampoline.asm
	$(NASM) -f bin -o trampoline.bin trampoline.asm

%.o: %.asm
	$(NASM) $(NASM_FLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

clean:
	rm -f $(ASM_OBJECTS) $(CPP_OBJECTS) trampoline.bin
