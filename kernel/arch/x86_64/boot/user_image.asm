; =============================================================================
; DIT-OS Kernel - User programs embedded in the kernel image
; The boot image is copied to the user window by init_user_image(); the second
; program is seeded into the root filesystem so execve() can load it.
; =============================================================================

BITS 32

section .rodata
global user_image_start
global user_image_end
user_image_start:
    incbin "../userland/build/init.bin"
user_image_end:

; The exec target has to be a real ELF, not a flat binary like the boot image,
; because execve reads its program headers.
global exec_image_start
global exec_image_end
exec_image_start:
    incbin "../userland/build/greeter.elf"
exec_image_end:
