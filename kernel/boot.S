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
.section .multiboot
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

.section .bss
/* stack */
.align 16
stack_bottom:
.skip STACK_SIZE
stack_top:

.section .text
.global _start
_start:
	/* initialize stack pointer */
	mov $stack_top, %esp

	/* reset eflags */
	pushl $0
	popf

	/* push pointer to multiboot information structure */
	pushl %ebx

	/* push magic value */
	pushl %eax

	/* entering the main function... */
	call kmain

	cli
loop:
	hlt
	jmp loop
