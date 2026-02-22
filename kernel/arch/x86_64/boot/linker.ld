OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

/* Separate LOAD segments: code RX, data RW (avoids RWX warning) */
PHDRS
{
    text PT_LOAD FLAGS(5);   /* PF_R | PF_X = read + execute */
    data PT_LOAD FLAGS(6);   /* PF_R | PF_W = read + write */
}

SECTIONS
{
    . = 0x100000;

    .multiboot : { *(.multiboot) } :text
    .multiboot2 : { *(.multiboot2) } :text
    .text : { *(.text) } :text
    .rodata : { *(.rodata) } :text

    .data : { *(.data) } :data
    .bss : {
        __bss_start = .;
        *(.bss)
        *(COMMON)
        __bss_end = .;
        __kernel_end = .;
    } :data
}
