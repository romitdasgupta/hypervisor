/* boot.S - Multiboot2 header and early boot code */

.set MAGIC,    0xE85250D6
.set ARCH,     0  /* i386 (protected mode) */
/* Header length: magic(4) + arch(4) + header_len(4) + checksum(4) + end_tag(8) = 24 */
.set HEADER_LEN, 24
.set CHECKSUM, -(MAGIC + ARCH + HEADER_LEN)

/* Multiboot header at the very beginning of .multiboot section */
.section .multiboot, "a"
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

/* Dedicated stack section (not in .bss, won't be cleared) */
.section .stack, "aw", @nobits
.align 16
stack_bottom:
    .skip 16384  /* 16 KiB stack */
stack_top:

.section .text
.code32
.global _start
.type _start, @function

_start:
    /* We're in 32-bit protected mode, need to switch to long mode */
    cli
    
    /* Set up stack */
    mov $stack_top, %esp
    
    /* Save multiboot info */
    pushl %ebx  /* Multiboot info structure */
    pushl %eax  /* Multiboot magic */
    
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
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4
    
    /* Load PML4 */
    movl $pml4, %eax
    movl %eax, %cr3
    
    /* Enable long mode in EFER MSR */
    movl $0xC0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr
    
    /* Load 64-bit GDT before enabling paging */
    lgdt gdt64_pointer

    /* Enable paging and protected mode */
    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0
    
    /* Far jump to 64-bit code */
    ljmpl $0x08, $long_mode_start

check_cpuid:
    pushfl
    popl %eax
    movl %eax, %ecx
    xorl $0x200000, %eax
    pushl %eax
    popfl
    pushfl
    popl %eax
    pushl %ecx
    popfl
    xorl %ecx, %eax
    ret

check_long_mode:
    movl $0x80000000, %eax
    cpuid
    cmpl $0x80000001, %eax
    jb no_long_mode_support
    
    movl $0x80000001, %eax
    cpuid
    testl $(1 << 29), %edx
    jz no_long_mode_support
    
    movl $1, %eax
    ret

no_long_mode_support:
    xorl %eax, %eax
    ret

setup_page_tables:
	/* Clear page tables */
	movl $pml4, %edi
	movl %edi, %cr3
	xorl %eax, %eax
	movl $4096, %ecx
	rep stosl
	movl %cr3, %edi

	/* PML4[0] -> PDPT */
	movl $pdpt, %eax
	orl $3, %eax			/* Present + Writable */
	movl %eax, pml4

	/* PDPT[0] -> PDT */
	movl $pdt, %eax
	orl $3, %eax
	movl %eax, pdpt

	/* Identity map first 2MB using 2MB pages */
	movl $pdt, %edi
	movl $0x83, (%edi)		/* Map first 2MB at address 0x0 */
	addl $8, %edi
	movl $0x200083, %eax		/* Present + Writable + Huge Page, starting at 2MB */
	movl $511, %ecx			/* 511 more entries (512 total - 1) */
1:
	movl %eax, (%edi)
	addl $0x200000, %eax
	addl $8, %edi
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
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Clear BSS section  */
    cld                     /* Good practice before rep stosx */
    movq $_bss_start, %rdi
    movq $_bss_end, %rcx
    subq %rdi, %rcx
    xorq %rax, %rax
    rep stosb


    /* Restore multiboot info (pop as 32-bit, extend to 64-bit) */
    xorq %rdi, %rdi
    xorq %rsi, %rsi
    movl (%rsp), %edi    /* multiboot magic (32-bit) */
    movl 4(%rsp), %esi   /* multiboot info (32-bit) */

    movq $stack_top, %rsp /* reset the stack */

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
    .quad 0x00209A0000000000     /* Code segment: L=1, P=1, S=1, E=1, RW=1 */
    .quad 0x0000920000000000     /* Data segment: P=1, S=1, RW=1 */
gdt64_pointer:
    .word . - gdt64 - 1
    .quad gdt64

/* Page tables in separate section (not cleared) */
.section .page_tables, "aw", @nobits
.align 4096
pml4:
    .skip 4096
pdpt:
    .skip 4096
pdt:
    .skip 4096

/* BSS section for uninitialized data */
.section .bss
.align 16

/* Mark stack as non-executable */
.section .note.GNU-stack,"",@progbits
