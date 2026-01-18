ENTRY(_Start)

PHDRS
{
    text PT_LOAD FLAGS(5);
}

SECTIONS
{
    . = 0x9FC00000;

    .text : AT(0x9FC00000) {
        *(.text*)
        *(.rodata*)
    } :text

    .data : AT(0x9FC00000 + SIZEOF(.text)) {
        *(.data*)
    }

    .bss : {
        *(.bss*)
        *(COMMON)
    }
}