cmake_minimum_required(VERSION 3.10)
project(DIT-OS_kernel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KERNEL_DIR ${CMAKE_SOURCE_DIR}/kernel)
set(ARCH_DIR ${KERNEL_DIR}/arch/x86_64)
set(BOOT_DIR ${ARCH_DIR}/boot)
set(CPU_DIR ${ARCH_DIR}/cpu)
set(INCLUDE_DIR ${KERNEL_DIR}/include)

set(KERNEL_OUTPUT ${CMAKE_SOURCE_DIR}/bootloader/build/kernel.bin)

set(ASM_SOURCES
    ${BOOT_DIR}/multiboot.asm
    ${BOOT_DIR}/entry.asm
    ${CPU_DIR}/gdt.asm
    ${CPU_DIR}/idt.asm
)

set(CPP_SOURCES
    ${KERNEL_DIR}/main.cpp
    ${ARCH_DIR}/serial.cpp
    ${ARCH_DIR}/printk.cpp
)

set(ASM_OBJECTS)
foreach(src ${ASM_SOURCES})
    get_filename_component(name ${src} NAME)
    list(APPEND ASM_OBJECTS ${CMAKE_BINARY_DIR}/${name}.o)
endforeach()

set(CPP_OBJECTS)
foreach(src ${CPP_SOURCES})
    get_filename_component(name ${src} NAME)
    string(REPLACE ".cpp" ".o" obj_name ${name})
    list(APPEND CPP_OBJECTS ${CMAKE_BINARY_DIR}/${obj_name})
endforeach()

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

foreach(src ${ASM_SOURCES})
    get_filename_component(name ${src} NAME)
    set(obj ${CMAKE_BINARY_DIR}/${name}.o)
    add_custom_command(
        OUTPUT ${obj}
        COMMAND nasm -f elf32 -g -o ${obj} ${src}
        COMMENT "Assembling ${src}"
        DEPENDS ${src}
        VERBATIM
    )
endforeach()

foreach(src ${CPP_SOURCES})
    get_filename_component(name ${src} NAME)
    string(REPLACE ".cpp" ".o" obj_name ${name})
    set(obj ${CMAKE_BINARY_DIR}/${obj_name})
    add_custom_command(
        OUTPUT ${obj}
        COMMAND g++ -std=c++17 -ffreestanding -fno-rtti -fno-exceptions -Wall -Wextra -m32 -I${INCLUDE_DIR} -c -o ${obj} ${src}
        COMMENT "Compiling ${src}"
        DEPENDS ${src}
        VERBATIM
    )
endforeach()
