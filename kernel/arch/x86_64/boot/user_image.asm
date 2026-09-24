BITS 32

section .rodata
global user_image_start
global user_image_end
user_image_start:
    incbin "../userland/build/init.bin"
user_image_end:
