ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 1MB physical address */
    . = 1M;

    .boot :
    {
        *(.multiboot)
    }

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

    .bss ALIGN(4K) : AT(ADDR(.bss))
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Discard unnecessary sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
