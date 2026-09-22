# DIT-OS Kernel Makefile

# Toolchain: use the i686-elf cross-compiler when it is installed, otherwise
# the host GCC in 32-bit mode. Override with e.g. `make CROSS_COMPILE=`.
CROSS_COMPILE ?= $(if $(shell command -v i686-elf-g++ 2>/dev/null),i686-elf-,)
CXX = $(CROSS_COMPILE)g++
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
NASM = nasm

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
GFX_DIR = $(ARCH_DIR)/graphics
CORE_DIR = core
SYSCALL_DIR = syscall
SCHED_DIR = sched
FS_DIR = fs

# Assembly sources
ASM_SOURCES = $(BOOT_DIR)/multiboot.asm $(BOOT_DIR)/multiboot2.asm $(BOOT_DIR)/entry.asm \
	$(CPU_DIR)/gdt.asm $(CPU_DIR)/idt.asm $(CPU_DIR)/tss.asm $(CPU_DIR)/pic.asm \
	$(CPU_DIR)/msr.asm $(CPU_DIR)/cpuid.asm $(CPU_DIR)/control.asm $(CPU_DIR)/smp.asm \
	$(MM_DIR)/paging.asm $(MM_DIR)/tlb.asm $(MM_DIR)/mtrr.asm \
	$(SCHED_DIR)/context.asm

# Core (included from core/Makefile)
include $(CORE_DIR)/Makefile

# C++ sources
CPP_SOURCES = $(BOOT_DIR)/init.cpp $(BOOT_DIR)/main.cpp \
	$(CORE_SOURCES) \
	$(SYSCALL_DIR)/syscall_table.cpp $(SYSCALL_DIR)/syscall.cpp $(SYSCALL_DIR)/syscall_trace.cpp \
	$(SYSCALL_DIR)/fd.cpp \
	$(SYSCALL_DIR)/sys_read.cpp $(SYSCALL_DIR)/sys_write.cpp $(SYSCALL_DIR)/sys_exit.cpp \
	$(SYSCALL_DIR)/sys_open.cpp $(SYSCALL_DIR)/sys_close.cpp $(SYSCALL_DIR)/sys_getdents.cpp \
	$(SYSCALL_DIR)/sys_getpid.cpp $(SYSCALL_DIR)/sys_fork.cpp \
	$(SYSCALL_DIR)/sys_ioctl.cpp $(SYSCALL_DIR)/tty.cpp \
	$(SYSCALL_DIR)/syscall_wrappers.cpp \
	$(SYSCALL_DIR)/userland_syscall_cwrap.cpp \
	$(FS_DIR)/vfs.cpp $(FS_DIR)/ramfs/ramfs.cpp $(FS_DIR)/ramfs/inode.cpp \
	$(SCHED_DIR)/sched.cpp $(SCHED_DIR)/task.cpp $(SCHED_DIR)/idle.cpp $(SCHED_DIR)/init_task.cpp $(SCHED_DIR)/shell.cpp \
	$(SCHED_DIR)/pid.cpp $(SCHED_DIR)/exit.cpp $(SCHED_DIR)/fork.cpp $(SCHED_DIR)/mutex.cpp \
	$(CPU_DIR)/gdt.cpp $(CPU_DIR)/idt.cpp $(CPU_DIR)/tss.cpp $(CPU_DIR)/pic.cpp \
	$(CPU_DIR)/msr.cpp $(CPU_DIR)/cpuid.cpp $(CPU_DIR)/control.cpp \
	$(CPU_DIR)/apic.cpp $(CPU_DIR)/lapic.cpp $(CPU_DIR)/ioapic.cpp $(CPU_DIR)/smp.cpp \
	$(CPU_DIR)/mtrr.cpp $(CPU_DIR)/pat.cpp $(CPU_DIR)/cpufeatures.cpp $(CPU_DIR)/arch.cpp \
	$(MM_DIR)/paging.cpp $(MM_DIR)/pmm.cpp $(MM_DIR)/vmm.cpp $(MM_DIR)/heap.cpp $(MM_DIR)/dma.cpp \
	$(MM_DIR)/slab.cpp $(MM_DIR)/memblock.cpp $(MM_DIR)/early_mm.cpp \
	$(MM_DIR)/uaccess.cpp \
	$(DEV_DIR)/serial.cpp $(DEV_DIR)/keyboard.cpp $(DEV_DIR)/mouse.cpp $(DEV_DIR)/timer.cpp $(DEV_DIR)/rtc.cpp \
	$(DEV_DIR)/pci.cpp $(DEV_DIR)/ata.cpp $(DEV_DIR)/ahci.cpp $(DEV_DIR)/nvme.cpp $(DEV_DIR)/floppy.cpp \
	$(GFX_DIR)/vga.cpp $(GFX_DIR)/framebuffer.cpp

# C sources (userland code built into the kernel for now)
C_SOURCES = \
	../userland/bin/init.c \
	../userland/shell/main.c \
	../userland/shell/readline.c \
	../userland/libc/src/stdlib/malloc.c \
	../userland/libc/src/stdlib/exit.c \
	../userland/libc/src/stdio/printf.c

# Object files in OBJDIR to avoid circular deps and name clashes
ASM_OBJECTS = $(addprefix $(OBJDIR)/,$(patsubst %.asm,%.asm.o,$(ASM_SOURCES)))
CPP_OBJECTS = $(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(CPP_SOURCES)))
# ../userland/x.c -> build/userland/x.o (keeps userland objects inside build/)
C_OBJECTS = $(patsubst ../%.c,$(OBJDIR)/%.o,$(C_SOURCES))

# Flags
NASM_FLAGS = -f elf32 -g
# Shared by C and C++: warnings are errors, and -MMD -MP writes .d files so
# header changes trigger rebuilds. -march=i686 -mgeneral-regs-only pins the
# ISA: some compilers (e.g. Nix's GCC) emit SSE for -m32 by default, and the
# kernel neither enables SSE nor saves FPU/SSE state on context switch.
COMMON_FLAGS = -ffreestanding -fno-stack-protector -fno-pie -fno-pic -m32 \
	-march=i686 -mgeneral-regs-only -O2 -Wall -Wextra -Werror -MMD -MP
CXX_FLAGS = -std=c++17 -fno-rtti -fno-exceptions $(COMMON_FLAGS)
CXX_FLAGS += -I$(KERNEL_DIR)/include -I../include

C_FLAGS = -std=c11 $(COMMON_FLAGS)
C_FLAGS += -I$(KERNEL_DIR)/include -I../include
C_FLAGS += -I../userland/include -I../userland/libc/include

# Prevent implicit %.asm: %.asm.o (avoids wrong build when updating .asm prerequisite)
%.asm: ;

# Default target
.PHONY: all
all: $(KERNEL_OUT)

# Build kernel
$(KERNEL_OUT): $(ASM_OBJECTS) $(CPP_OBJECTS) $(C_OBJECTS)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking kernel..."
	$(LD) -m elf_i386 -T $(BOOT_DIR)/linker.ld -z noexecstack -o $@ $(ASM_OBJECTS) $(CPP_OBJECTS) $(C_OBJECTS)
	@echo "Kernel built: $@"

# Assemble .asm files (obj dir avoids circular dependency)
$(OBJDIR)/%.asm.o: %.asm Makefile
	@mkdir -p $(dir $@)
	@echo "Assembling $<..."
	$(NASM) $(NASM_FLAGS) -o $@ $<

# Compile .cpp files
$(OBJDIR)/%.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

# Compile .c files
$(OBJDIR)/userland/%.o: ../userland/%.c Makefile
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(C_FLAGS) -c -o $@ $<

-include $(CPP_OBJECTS:.o=.d) $(C_OBJECTS:.o=.d)

# Clean
.PHONY: clean
clean:
	rm -rf $(OBJDIR) $(KERNEL_OUT)

.PHONY: dirs
dirs:
	@mkdir -p $(BUILD_DIR)
