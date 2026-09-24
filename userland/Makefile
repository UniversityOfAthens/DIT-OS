CROSS_COMPILE ?= $(if $(shell command -v i686-elf-gcc 2>/dev/null),i686-elf-,)
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
NASM = nasm

BUILD_DIR = build
C_SOURCES = \
	bin/init.c \
	shell/main.c \
	shell/readline.c \
	libc/src/stdlib/malloc.c \
	libc/src/stdlib/exit.c \
	libc/src/stdio/printf.c
C_OBJECTS = $(addprefix $(BUILD_DIR)/,$(C_SOURCES:.c=.o))
ASM_OBJECTS = $(BUILD_DIR)/crt0.o

CFLAGS = -m32 -std=c11 -ffreestanding -fno-stack-protector -fno-pie \
	-fno-pic -mgeneral-regs-only -O2 -Wall -Wextra -Werror -MMD -MP \
	-Iinclude -Ilibc/include -Ishell
LDFLAGS = -m32 -no-pie -nostdlib -nodefaultlibs -nostartfiles \
	-Wl,--build-id=none -Wl,-T,linker.ld -Wl,-z,noexecstack

.PHONY: all clean
all: $(BUILD_DIR)/init.elf $(BUILD_DIR)/init.bin

$(BUILD_DIR)/init.elf: $(ASM_OBJECTS) $(C_OBJECTS) linker.ld
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(C_OBJECTS)

$(BUILD_DIR)/init.bin: $(BUILD_DIR)/init.elf
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
