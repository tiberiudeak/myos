.set ALIGN,			1<<0             /* align boot modules on page boundaries */
.set MEMINFO,		1<<1             /* provide memory map */
.set VIDEOINFO,		1<<2			 /* provide video info */
.set FLAGS,			ALIGN | MEMINFO | VIDEOINFO
.set MAGIC,			0x1BADB002       /* magic number identifying the header */
.set CHECKSUM,		-(MAGIC + FLAGS)
.set STACK_SIZE,	0x4000			 /* set stack size to 16 KiB */

/**
 * Multiboot Header
 *
 * The first 12 bytes represent the magic fields of the multiboot header,
 * comprising of the magic number, the flags and their checksum.
 * The next 20 bytes form the address fields of the header, which are
 * not relevant for us.
 * The next 16 bytes represent the graphics fields, which specify the
 * preferred graphics mode. The order is mode_type, width, height and depth.
 * They are enabled by setting bit 2 in the flags.
 *
 * The multiboot header has to be contained in the first 8192 bytes
 * of the image (the header is put first in the linker script) and be
 * 32-bit aligned.
 */
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0			/* linear graphics mode */
.long 1024		/* number of columns in pixels */
.long 768		/* number of lines in pixels */
.long 32		/* bits per pixel */


/* initial stack */
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip STACK_SIZE
stack_top:

.section .bss, "aw", @nobits
.align 4096
.global boot_page_directory
// reserve memory for the page directory
boot_page_directory:
.skip 4096
// reserve memory for the page tables
.global boot_page_table1
boot_page_table1:
.skip 4096

.extern _kernel_end
.section .multiboot.text, "a"
.global _start
_start:
	/* Initial Page Directory & Page Table initialization */

	/* Structure of the initial PD and PT


	|-----------------|         |-----------------|             |                 |
1023| pd_addr | flags |         | 0x3FF000| flags |             |                 |
	|-----------------|         |-----------------|             |                 |
	|-----------------|         |                 |             |-----------------| 0x400000
768	| pt_addr | flags |----     |                 |             |                 |
	|-----------------|   |     |       ...       |             |                 |
	|                 |   |     |                 |             |                 |
	|                 |   |     |                 |             |                 |
	|       ...       |   |     |-----------------|             |                 |
	|                 |   |     | 0x02000 | flags |             |       4MB       |
	|                 |   |     |-----------------|             |                 |
	|                 |   |     | 0x01000 | flags |             |                 |
	|-----------------|   |     |-----------------|             |                 |
0	| pt_addr | flags |----     | 0x00000 | flags |             |                 |
	|-----------------|   |---->|-----------------|             |-----------------| 0x000000
	   Page Directory                Page Table                          RAM

	*/

	/* fill in the page directory
		- identity map the first 4MB
		- also map first 4MB at 0xC0000000
		- map page directory to itself
	 */

	// set the entries at indices 0 and 0x300 (corresponding to virtual address 0xC0000000)
	// to the corresponding page table address (and also set the present and read-write bits)
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000
	movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0x300 * 4

	// map the page directory itself into its last entry
	movl $(boot_page_directory - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0x3FF * 4

	/* fill in the page table */

	// initialize %edi with the address of the page table
	// and %esi with the first address of the 4KiB page frame
	// (which is 0x00000000)
	movl $(boot_page_table1 - 0xC0000000), %edi
	movl $0, %esi

	// map only until _kernel_end
	movl $_kernel_end, %ecx
	// skip offset
	shrl $12, %ecx
	// mask page directory index
	andl $0x3FF, %ecx

1:
	// move 4KiB page frame address into %edx and set the
	// less two sognificant bits (marking the page as present
	// and read-write) - it's true, all pages are now set as
	// read-write, so even the .text section if it's in the
	// first 4MB. This will be changed later in the vmm
	movl %esi, %edx
	orl $0x003, %edx
	movl %edx, (%edi)

	// go to the address of the next 4kb page frame
	// and to the next position in the page table
	addl $4096, %esi
	addl $4, %edi

	loop 1b

2:
	// set %cr3 to the physical address of the page table
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	// enable paging and the write-protect bit
	movl %cr0, %ecx
	orl $0x80000001, %ecx
	movl %ecx, %cr0

	// jump to higher half
	lea 3f, %ecx
	jmp *%ecx

.section .text

3:
	// initialize stack pointer
	mov $stack_top, %esp

	// reset eflags
	pushl $0
	popf

	// push pointer to multiboot information structure
	pushl %ebx

	// push magic value
	pushl %eax

	// entering the main function...
	call kmain

	// endless loop, should not be reached
	cli
loop:
	hlt
	jmp loop
