cmake_minimum_required(VERSION 3.10)
project(DIT-OS LANGUAGES CXX)

# DIT-OS: custom bootloader (stage1 + stage2) + kernel.
# This file builds the kernel only. Use bootloader/Makefile for disk image and run.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Paths (match kernel/Makefile layout)
set(KERNEL_DIR ${CMAKE_SOURCE_DIR}/kernel)
set(ARCH_DIR ${KERNEL_DIR}/arch/x86_64)
set(BOOT_DIR ${ARCH_DIR}/boot)
set(CPU_DIR ${ARCH_DIR}/cpu)
set(INCLUDE_DIR ${KERNEL_DIR}/include)

# Kernel output: bootloader build dir (for disk image)
set(KERNEL_OUTPUT ${CMAKE_SOURCE_DIR}/bootloader/build/kernel.bin)

# Assembly sources (multiboot, entry, GDT, IDT)
set(ASM_SOURCES
    ${BOOT_DIR}/multiboot.asm
    ${BOOT_DIR}/multiboot2.asm
    ${BOOT_DIR}/entry.asm
    ${CPU_DIR}/gdt.asm
    ${CPU_DIR}/idt.asm
)

# C++ sources (early init + main)
set(CPP_SOURCES
    ${BOOT_DIR}/init.cpp
    ${BOOT_DIR}/main.cpp
)

# Object file names (in build dir; NAME_WE = base name without extension)
set(ASM_OBJECTS)
foreach(src ${ASM_SOURCES})
    get_filename_component(name_we ${src} NAME_WE)
    list(APPEND ASM_OBJECTS ${CMAKE_BINARY_DIR}/${name_we}.o)
endforeach()

set(CPP_OBJECTS)
foreach(src ${CPP_SOURCES})
    get_filename_component(name_we ${src} NAME_WE)
    list(APPEND CPP_OBJECTS ${CMAKE_BINARY_DIR}/${name_we}.o)
endforeach()

# Include directories for C++ (match kernel Makefile)
set(KERNEL_CXX_FLAGS -I${KERNEL_DIR}/include -I${CMAKE_SOURCE_DIR}/include)

# Custom command: link kernel ELF
add_custom_command(
    OUTPUT ${KERNEL_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -E echo "Linking kernel..."
    COMMAND ld -m elf_i386 -T ${BOOT_DIR}/linker.ld -o ${KERNEL_OUTPUT} ${ASM_OBJECTS} ${CPP_OBJECTS}
    COMMAND ${CMAKE_COMMAND} -E echo "Kernel built: ${KERNEL_OUTPUT}"
    DEPENDS ${ASM_OBJECTS} ${CPP_OBJECTS}
    COMMENT "Linking kernel"
    VERBATIM
)

add_custom_target(kernel ALL DEPENDS ${KERNEL_OUTPUT})

# Assemble .asm
foreach(src ${ASM_SOURCES})
    get_filename_component(name_we ${src} NAME_WE)
    set(obj ${CMAKE_BINARY_DIR}/${name_we}.o)
    add_custom_command(
        OUTPUT ${obj}
        COMMAND nasm -f elf32 -g -o ${obj} ${src}
        COMMENT "Assembling ${name_we}.asm"
        DEPENDS ${src}
        VERBATIM
    )
endforeach()

# Compile .cpp (ffreestanding, no RTTI/exceptions, 32-bit)
foreach(src ${CPP_SOURCES})
    get_filename_component(name_we ${src} NAME_WE)
    set(obj ${CMAKE_BINARY_DIR}/${name_we}.o)
    add_custom_command(
        OUTPUT ${obj}
        COMMAND g++ -std=c++17 -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -m32 ${KERNEL_CXX_FLAGS} -c -o ${obj} ${src}
        COMMENT "Compiling ${name_we}.cpp"
        DEPENDS ${src}
        VERBATIM
    )
endforeach()
