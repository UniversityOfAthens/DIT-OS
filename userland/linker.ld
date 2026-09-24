OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    . = 0x00400000;

    /DISCARD/ : { *(.note.gnu.build-id) }

    .text : { *(.text .text.*) } :text
    .rodata : { *(.rodata .rodata.*) } :text

    . = ALIGN(0x1000);
    .data : { *(.data .data.*) } :data
    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    } :data
}
