ENTRY (_start)
OUTPUT_ARCH(i386:x86-64)

KERNEL_VIRT_BASE_ADDR = 0xffffffff80000000;
KERNEL_PHYS_BASE_ADDR = 0x100000;

/* the memory limit is currently 2MB (so kernel 1MB) */
MEMORY_LIMIT = 0x200000;

PHDRS
{
	multiboot_text PT_LOAD FLAGS(5);
	multiboot_data PT_LOAD FLAGS(4);
	text           PT_LOAD FLAGS(5);
	rodata         PT_LOAD FLAGS(4);
	data           PT_LOAD FLAGS(6);
}

SECTIONS
{
	/* load kernel above 1MB */
    . = KERNEL_PHYS_BASE_ADDR;

	/* start with the multiboot header, followed by the
	   initial code that initializes the 4level paging
	   structures and makes the switch to 64bit mode
	 */
	_kernel_start = .;
	.multiboot.data ALIGN(4K) : {
		*(.multiboot.data)
	} :multiboot_data

   .multiboot.text ALIGN(4K) : {
	   *(.multiboot.text)
   } :multiboot_text

	. += KERNEL_VIRT_BASE_ADDR;

	.text ALIGN (4K) : AT (ADDR (.text) - KERNEL_VIRT_BASE_ADDR)
	{
		*(.text)
	} :text

	.rodata ALIGN (4K) : AT (ADDR (.rodata) - KERNEL_VIRT_BASE_ADDR)
	{
		*(.rodata)
	} :rodata

	.data ALIGN (4K) : AT (ADDR (.data) - KERNEL_VIRT_BASE_ADDR)
	{
		*(.data)
	} :data

	.bss ALIGN (4K) : AT (ADDR (.bss) - KERNEL_VIRT_BASE_ADDR)
	{
		*(.bss)
		*(COMMON)
	} :data
	. = ALIGN(4K);
	_kernel_end = .;

	_kernel_start_phys = KERNEL_PHYS_BASE_ADDR;
	_kernel_end_phys = LOADADDR(.bss) + SIZEOF(.bss);
	_kernel_size = _kernel_end_phys - _kernel_start_phys;

	/* as of right now, in the early boot we reserve and initialize page tables statically,
	   so the kernel size has a limit. If this is exceeded, additional page tables should
	   be added in the early boot to cover the entire kernel
	 */
	ASSERT(_kernel_end_phys <= MEMORY_LIMIT,
         "Error: kernel image size exceeds the physical limit! Modify the boot paging structures to accommodate the kernel size!")
}
