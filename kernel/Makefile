# DIT-OS Kernel Makefile

# Toolchain
CXX = g++
NASM = nasm
LD = ld

# Directories
BUILD_DIR = ../bootloader/build
KERNEL_OUT = $(BUILD_DIR)/kernel.bin

# Architecture
ARCH = x86_64

# Source directories
KERNEL_DIR = .
ARCH_DIR = arch/$(ARCH)
BOOT_DIR = $(ARCH_DIR)/boot
CPU_DIR = $(ARCH_DIR)/cpu
MM_DIR = $(ARCH_DIR)/mm

# Assembly sources
ASM_SOURCES = $(BOOT_DIR)/multiboot.asm $(BOOT_DIR)/entry.asm $(CPU_DIR)/gdt.asm $(CPU_DIR)/idt.asm

# C++ sources  
CPP_SOURCES = $(BOOT_DIR)/main.cpp

# Object files
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
CPP_OBJECTS = $(CPP_SOURCES:.cpp=.o)

# Flags
NASM_FLAGS = -f elf32 -g
CXX_FLAGS = -std=c++17 -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -m32
CXX_FLAGS += -I../include -I../../include

# Default target
.PHONY: all
all: $(KERNEL_OUT)

# Build kernel
$(KERNEL_OUT): $(ASM_OBJECTS) $(CPP_OBJECTS)
	@echo "Linking kernel..."
	$(LD) -T $(BOOT_DIR)/linker.ld -o $@ $(ASM_OBJECTS) $(CPP_OBJECTS)
	@echo "Kernel built: $@"

# Assemble .asm files
%.o: %.asm
	@echo "Assembling $<..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# Compile .cpp files
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

# Clean
.PHONY: clean
clean:
	rm -f $(ASM_OBJECTS) $(CPP_OBJECTS) $(KERNEL_OUT)

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_DIR)
