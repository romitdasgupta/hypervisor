ENTRY(_start)

SECTIONS
{
    /* Multiboot header at the beginning of the file */
    . = 0;

    .multiboot :
    {
        *(.multiboot)
    }

    /* Kernel code and data starting at 1MB physical address */
    . = 1M;

    .text ALIGN(4K) : AT(ADDR(.text))
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata))
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) : AT(ADDR(.data))
    {
        *(.data)
        *(.data.*)
    }

    _bss_start = .;
    .bss ALIGN(4K) : AT(ADDR(.bss))
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
    _bss_end = .;

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
