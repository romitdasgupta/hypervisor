/* boot.S - Multiboot2 header and early boot code */

.set MAGIC,    0xE85250D6
.set ARCH,     0  /* i386 (protected mode) */
.set HEADER_LEN, (multiboot_end - multiboot_start)
.set CHECKSUM, -(MAGIC + ARCH + HEADER_LEN)

.section .multiboot
.align 8
multiboot_start:
    .long MAGIC
    .long ARCH
    .long HEADER_LEN
    .long CHECKSUM
    
    /* End tag */
    .align 8
    .short 0
    .short 0
    .long 8
multiboot_end:

.section .bss
.align 16
stack_bottom:
    .skip 16384  /* 16 KiB stack */
stack_top:

.section .text
.global _start
.type _start, @function

_start:
    /* We're in 32-bit protected mode, need to switch to long mode */
    cli
    
    /* Set up stack */
    mov $stack_top, %esp
    
    /* Save multiboot info */
    push %ebx  /* Multiboot info structure */
    push %eax  /* Multiboot magic */
    
    /* Check CPUID support */
    call check_cpuid
    test %eax, %eax
    jz no_cpuid
    
    /* Check long mode support */
    call check_long_mode
    test %eax, %eax
    jz no_long_mode
    
    /* Set up page tables for identity mapping */
    call setup_page_tables
    
    /* Enable PAE */
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4
    
    /* Load PML4 */
    mov $pml4, %eax
    mov %eax, %cr3
    
    /* Enable long mode in EFER MSR */
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr
    
    /* Enable paging and protected mode */
    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0
    
    /* Load 64-bit GDT */
    lgdt gdt64_pointer
    
    /* Far jump to 64-bit code */
    ljmp $0x08, $long_mode_start

check_cpuid:
    pushfl
    pop %eax
    mov %eax, %ecx
    xor $0x200000, %eax
    push %eax
    popfl
    pushfl
    pop %eax
    push %ecx
    popfl
    xor %ecx, %eax
    ret

check_long_mode:
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb no_long_mode_support
    
    mov $0x80000001, %eax
    cpuid
    test $(1 << 29), %edx
    jz no_long_mode_support
    
    mov $1, %eax
    ret

no_long_mode_support:
    xor %eax, %eax
    ret

setup_page_tables:
    /* Clear page tables */
    mov $pml4, %edi
    mov %edi, %cr3
    xor %eax, %eax
    mov $4096, %ecx
    rep stosl
    mov %cr3, %edi
    
    /* PML4[0] -> PDPT */
    mov $pdpt, %eax
    or $3, %eax  /* Present + Writable */
    mov %eax, pml4
    
    /* PDPT[0] -> PDT */
    mov $pdt, %eax
    or $3, %eax
    mov %eax, pdpt
    
    /* Identity map first 2MB using 2MB pages */
    mov $pdt, %edi
    mov $0x83, %eax  /* Present + Writable + Huge Page */
    mov $512, %ecx
1:
    mov %eax, (%edi)
    add $0x200000, %eax
    add $8, %edi
    loop 1b
    
    ret

no_cpuid:
no_long_mode:
    /* Hang */
    cli
    hlt
    jmp no_long_mode

.code64
long_mode_start:
    /* Clear segment registers */
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    /* Restore multiboot info */
    pop %rdi  /* multiboot magic */
    pop %rsi  /* multiboot info */
    
    /* Call C++ kernel main */
    call kernel_main
    
    /* Hang if kernel returns */
    cli
1:
    hlt
    jmp 1b

.section .data
.align 16
gdt64:
    .quad 0                      /* Null descriptor */
    .quad 0x00AF9A000000FFFF     /* Code segment */
    .quad 0x00AF92000000FFFF     /* Data segment */
gdt64_pointer:
    .word . - gdt64 - 1
    .quad gdt64

.section .bss
.align 4096
pml4:
    .skip 4096
pdpt:
    .skip 4096
pdt:
    .skip 4096