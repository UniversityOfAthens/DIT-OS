# DIT-OS Kernel Makefile

# Toolchain
CXX = g++
NASM = nasm
LD = ld

# Directories
BUILD_DIR = ../bootloader/build
OBJDIR = build
KERNEL_OUT = $(BUILD_DIR)/kernel.bin

# Architecture
ARCH = x86_64

# Source directories
KERNEL_DIR = .
ARCH_DIR = arch/$(ARCH)
BOOT_DIR = $(ARCH_DIR)/boot
CPU_DIR = $(ARCH_DIR)/cpu
MM_DIR = $(ARCH_DIR)/mm
DEV_DIR = $(ARCH_DIR)/dev
CORE_DIR = core

# Assembly sources
ASM_SOURCES = $(BOOT_DIR)/multiboot.asm $(BOOT_DIR)/multiboot2.asm $(BOOT_DIR)/entry.asm \
	$(CPU_DIR)/gdt.asm $(CPU_DIR)/idt.asm $(CPU_DIR)/tss.asm $(CPU_DIR)/pic.asm \
	$(CPU_DIR)/msr.asm $(CPU_DIR)/cpuid.asm $(CPU_DIR)/control.asm $(CPU_DIR)/smp.asm \
	$(MM_DIR)/paging.asm $(MM_DIR)/tlb.asm $(MM_DIR)/mtrr.asm

# Core (included from core/Makefile)
include $(CORE_DIR)/Makefile

# C++ sources
CPP_SOURCES = $(BOOT_DIR)/init.cpp $(BOOT_DIR)/main.cpp \
	$(CORE_SOURCES) \
	$(CPU_DIR)/gdt.cpp $(CPU_DIR)/idt.cpp $(CPU_DIR)/tss.cpp $(CPU_DIR)/pic.cpp \
	$(CPU_DIR)/msr.cpp $(CPU_DIR)/cpuid.cpp $(CPU_DIR)/control.cpp \
	$(CPU_DIR)/apic.cpp $(CPU_DIR)/lapic.cpp $(CPU_DIR)/ioapic.cpp $(CPU_DIR)/smp.cpp \
	$(CPU_DIR)/mtrr.cpp $(CPU_DIR)/pat.cpp $(CPU_DIR)/cpufeatures.cpp $(CPU_DIR)/arch.cpp \
	$(MM_DIR)/paging.cpp $(MM_DIR)/pmm.cpp $(MM_DIR)/vmm.cpp $(MM_DIR)/heap.cpp $(MM_DIR)/dma.cpp \
	$(MM_DIR)/slab.cpp $(MM_DIR)/memblock.cpp $(MM_DIR)/early_mm.cpp \
	$(DEV_DIR)/serial.cpp $(DEV_DIR)/keyboard.cpp $(DEV_DIR)/mouse.cpp $(DEV_DIR)/timer.cpp $(DEV_DIR)/rtc.cpp \
	$(DEV_DIR)/pci.cpp $(DEV_DIR)/ata.cpp $(DEV_DIR)/ahci.cpp $(DEV_DIR)/nvme.cpp $(DEV_DIR)/floppy.cpp

# Object files in OBJDIR to avoid circular deps and name clashes
ASM_OBJECTS = $(addprefix $(OBJDIR)/,$(patsubst %.asm,%.asm.o,$(ASM_SOURCES)))
CPP_OBJECTS = $(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(CPP_SOURCES)))

# Flags
NASM_FLAGS = -f elf32 -g
CXX_FLAGS = -std=c++17 -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -m32
CXX_FLAGS += -I$(KERNEL_DIR)/include -I../include

# Prevent implicit %.asm: %.asm.o (avoids wrong build when updating .asm prerequisite)
%.asm: ;

# Default target
.PHONY: all
all: $(KERNEL_OUT)

# Build kernel
$(KERNEL_OUT): $(ASM_OBJECTS) $(CPP_OBJECTS)
	@echo "Linking kernel..."
	$(LD) -m elf_i386 -T $(BOOT_DIR)/linker.ld -z noexecstack -o $@ $(ASM_OBJECTS) $(CPP_OBJECTS)
	@echo "Kernel built: $@"

# Assemble .asm files (obj dir avoids circular dependency)
$(OBJDIR)/%.asm.o: %.asm
	@mkdir -p $(dir $@)
	@echo "Assembling $<..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# Compile .cpp files
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

# Clean
.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(KERNEL_OUT)

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_DIR)
