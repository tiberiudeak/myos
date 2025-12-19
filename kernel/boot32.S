.set MAGIC,			0xE85250D6		/* magic number identifying the header */
.set ARCHITECTURE,	0				/* 0 = 32-bit prot mode of i386 */
.set HEADER_LENGTH,	24				/* length in bytes */
.set CHECKSUM,		-(MAGIC + ARCHITECTURE + HEADER_LENGTH)
.set STACK_SIZE,	0x4000			/* set stack size to 16 KiB */
.set K_VBASE,		0xffffffff80000000

/**
 * Multiboot 2 Header
 */
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long ARCHITECTURE
.long HEADER_LENGTH
.long CHECKSUM
.long 0
.long 8

.section .bss, "aw", @nobits
.align 4096
pml4:
.skip 4096
pdpt:
.skip 4096
pd:
.skip 4096
pt:
.skip 4096
stack_bottom:
.skip STACK_SIZE
stack_top:

.section .multiboot.text, "ax"
.global _start
.extern long_mode_entry
.code32
_start:
	cmp $0x36d76289, %eax
	// we're not loaded by multiboot2 compliant bootloader
	jne loop

	mov stack_top - K_VBASE, %esp
	
	call check_cpuid_support
	call check_longmode_support

	/* create 4-level paging structures */

	// pml4 entries
	movl $(pdpt - K_VBASE + 0x003), pml4 - K_VBASE
	movl $0, (pml4 - K_VBASE) + 4
	movl $(pdpt - K_VBASE + 0x003), pml4 - K_VBASE + 511 * 8
	movl $0, (pml4 - K_VBASE) + 511*8 + 4

	// pdpt entries
	movl $(pd - K_VBASE + 0x003), pdpt - K_VBASE
	movl $0, (pdpt - K_VBASE) + 4
	movl $(pd - K_VBASE + 0x003), pdpt - K_VBASE + 510 * 8
	movl $0, (pdpt - K_VBASE) + 510 * 8 + 4

	// pd entries
	movl $(pt - K_VBASE + 0x003), pd - K_VBASE
	movl $0, (pd - K_VBASE) + 4

	// pt entries (map first 2MB)
	movl $(pt - K_VBASE), %edi
	movl $0x0, %esi
	movl $512, %ecx

1:
	movl %esi, %edx
	or $0x003, %edx
	movl %edx, (%edi)

	addl $0x1000, %esi
	addl $8, %edi

	loop 1b

	lgdt gdt64_desc - K_VBASE

	/* initialize IA-32e mode */

	// 1. enable physical address extensions (PAE) (CR4.PAE = 1)
	movl %cr4, %eax
	orl $0x20, %eax
	movl %eax, %cr4

	// 2. load CR3 with the physical address of the PML4 table
	movl $(pml4 - K_VBASE), %eax
	movl %eax, %cr3

	// 3. enable IA-32e mode by setting IA32_EFER.LME = 1
	movl $0xC0000080, %ecx
	rdmsr
	orl $0x100, %eax
	wrmsr

	// 4. enable paging (CR0.PG = 1)
	movl %cr0, %eax
	orl $0x80000000, %eax
	movl %eax, %cr0

	ljmp $0x08, $(long_mode_entry - K_VBASE)

loop:
	hlt
	jmp loop

check_cpuid_support:
	/* bit 21 (ID) in the EFLAGS indicates support for the cpuid inst.
	if we can toggle this bit, then the processor supports it */
	pushfl
	pushfl
	xorl $0x00200000, (%esp)
	popfl
	pushfl
	pop %eax
	xor (%esp), %eax
	popfl
	and $0x00200000, %eax
	cmp $0, %eax
	je loop
	ret

check_longmode_support:
	/* check if extended processor info is available */
	mov $0x80000000, %eax
	cpuid
	cmp $0x80000001, %eax
	jl loop

	/* check if IA-32e mode is supported CPUID.80000001H:EDX[29] = 1 */
	mov $0x80000001, %eax
	cpuid
	and $0x20000000, %edx
	cmp $0, %edx
	je loop
	ret

.section .rodata
.align 8
gdt64:
	.quad 0x0000000000000000
	.quad 0x00AF9A0000000000
gdt64_end:
.align 8
gdt64_desc:
	.word gdt64_end - gdt64 - 1
	.long gdt64 - K_VBASE
