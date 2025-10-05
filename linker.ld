ENTRY(_start)

SECTIONS
{
    /* Kernel code and data starting at 1MB physical address */
    . = 1M;

    /* Multiboot header must be first in the loadable segment */
    .multiboot ALIGN(4):
    {
        KEEP(*(.multiboot))
        KEEP(*(.multiboot_header))
    }

    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    }

    /* C++ constructor tables */
    .init_array ALIGN(8) :
    {
        __init_array_start = .;
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    }

    /* Stack section (separate from BSS, won't be cleared) */
    .stack ALIGN(4K) :
    {
        *(.stack)
    }

    /* Page tables section (separate from BSS, won't be cleared) */
    .page_tables ALIGN(4K) :
    {
        *(.page_tables)
    }

    /* BSS section for uninitialized data that should be zeroed */
    _bss_start = .;
    .bss ALIGN(4K) :
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
