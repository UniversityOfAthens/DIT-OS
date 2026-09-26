CROSS_COMPILE ?= $(if $(shell command -v i686-elf-gcc 2>/dev/null),i686-elf-,)
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NASM = nasm

BUILD_DIR = build
# Code shared by every program.
LIBC_SOURCES = \
	libc/src/stdlib/malloc.c \
	libc/src/stdlib/exit.c \
	libc/src/stdio/printf.c
# init is the boot image (it also carries the shell); greeter is execve'd by
# the init self-test and needs no shell.
PROGRAMS = init greeter
C_SOURCES = bin/init.c bin/greeter.c shell/main.c shell/readline.c $(LIBC_SOURCES)
C_OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
# crt0.o comes first in the link order: it holds _start, and the flat binary
# starts at the lowest linked address, so it must be the image entry.
ASM_OBJECTS = $(BUILD_DIR)/crt0.o
COMMON_OBJECTS = $(ASM_OBJECTS) \
	$(BUILD_DIR)/libc/src/stdio/printf.o $(BUILD_DIR)/libc/src/stdlib/exit.o
INIT_OBJECTS = $(COMMON_OBJECTS) $(BUILD_DIR)/bin/init.o \
	$(BUILD_DIR)/shell/main.o $(BUILD_DIR)/shell/readline.o \
	$(BUILD_DIR)/libc/src/stdlib/malloc.o
GREETER_OBJECTS = $(COMMON_OBJECTS) $(BUILD_DIR)/bin/greeter.o
BINS = $(addprefix $(BUILD_DIR)/,$(addsuffix .bin,$(PROGRAMS)))

CFLAGS = -m32 -std=c11 -ffreestanding -fno-stack-protector -fno-pie \
	-fno-pic -mgeneral-regs-only -O2 -Wall -Wextra -Werror -MMD -MP \
	-Iinclude -Ilibc/include -Ishell
LDFLAGS = -m32 -no-pie -nostdlib -nodefaultlibs -nostartfiles \
	-Wl,--build-id=none -Wl,-T,linker.ld -Wl,-z,noexecstack

.PHONY: all clean
all: $(BINS)

$(BUILD_DIR)/init.elf: $(INIT_OBJECTS) linker.ld
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $(INIT_OBJECTS)

$(BUILD_DIR)/greeter.elf: $(GREETER_OBJECTS) linker.ld
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $(GREETER_OBJECTS)

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@mkdir -p $(dir $@)
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.asm Makefile
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 -g -o $@ $<

-include $(C_OBJECTS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)
