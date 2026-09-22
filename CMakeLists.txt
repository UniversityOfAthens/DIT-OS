cmake_minimum_required(VERSION 3.10)

# DIT-OS: thin CMake wrapper around the Makefiles, which are the single source
# of truth for sources, flags and toolchain choice (kernel/Makefile picks
# i686-elf-* when installed, otherwise host GCC with -m32).
#
#   cmake -S . -B build
#   cmake --build build                  # kernel + bootloader + disk image
#   cmake --build build --target run     # boot in QEMU
project(DIT-OS NONE)

find_program(GNU_MAKE NAMES gmake make)
if(NOT GNU_MAKE)
    message(FATAL_ERROR "GNU make is required to build DIT-OS")
endif()

set(KERNEL_DIR ${CMAKE_SOURCE_DIR}/kernel)
set(BOOTLOADER_DIR ${CMAKE_SOURCE_DIR}/bootloader)

add_custom_target(kernel
    COMMAND ${GNU_MAKE} -C ${KERNEL_DIR}
    COMMENT "Building kernel (bootloader/build/kernel.bin)"
    USES_TERMINAL
)

add_custom_target(disk ALL
    COMMAND ${GNU_MAKE} -C ${BOOTLOADER_DIR} all
    COMMENT "Building bootloader and disk image (bootloader/build/disk.img)"
    DEPENDS kernel
    USES_TERMINAL
)

add_custom_target(run
    COMMAND ${GNU_MAKE} -C ${BOOTLOADER_DIR} run
    DEPENDS disk
    USES_TERMINAL
)

add_custom_target(debug
    COMMAND ${GNU_MAKE} -C ${BOOTLOADER_DIR} debug
    DEPENDS disk
    USES_TERMINAL
)

add_custom_target(os-clean
    COMMAND ${GNU_MAKE} -C ${KERNEL_DIR} clean
    COMMAND ${GNU_MAKE} -C ${BOOTLOADER_DIR} clean
    COMMENT "Removing kernel and bootloader build outputs"
)
