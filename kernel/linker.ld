ENTRY (_start)

KERNEL_VIRT_BASE_ADDR = 0xC0000000;
KERNEL_PHYS_BASE_ADDR = 0x00100000;

PAGE_TABLES_EARLY = 1; /* modify if you add extra PTs */
PAGE_SIZE = 4096;
ENTRIES_PER_PT = 1024;
MEMORY_LIMIT = PAGE_TABLES_EARLY * PAGE_SIZE * ENTRIES_PER_PT;

SECTIONS
{
	/* load kernel above 1MB */
    . = KERNEL_PHYS_BASE_ADDR;

	/* start with the multiboot header, followed by the
	   initial code that initializes the page directory
	   and page table(s) needed to map the higher half kernel
	 */
	_kernel_start = .;
	.multiboot.data : {
		*(.multiboot.data)
	}

   .multiboot.text : {
	   *(.multiboot.text)
   }

	. += KERNEL_VIRT_BASE_ADDR;

	.text ALIGN (4K) : AT (ADDR (.text) - KERNEL_VIRT_BASE_ADDR)
	{
		_kernel_text_sec_start = .;
		*(.text)
		_kernel_text_sec_end = .;
	}

	.rodata ALIGN (4K) : AT (ADDR (.rodata) - KERNEL_VIRT_BASE_ADDR)
	{
		_kernel_rodata_sec_start = .;
		*(.rodata)
		_kernel_rodata_sec_end = .;
	}

	.data ALIGN (4K) : AT (ADDR (.data) - KERNEL_VIRT_BASE_ADDR)
	{
		*(.data)
	}

	.bss ALIGN (4K) : AT (ADDR (.bss) - KERNEL_VIRT_BASE_ADDR)
	{
		*(COMMON)
		*(.bss)

		. = ALIGN(4K);
		*(.bootstrap_stack)
	}
	_kernel_end = .;

	_kernel_start_phys = KERNEL_PHYS_BASE_ADDR;
	_kernel_end_phys = LOADADDR(.bss) + SIZEOF(.bss);
	_kernel_size = _kernel_end_phys - _kernel_start_phys;

	/* as of right now, in the early boot we reserve and initialize page tables statically,
	   so the kernel size has a limit. If this is exceeded, additional page tables should
	   be added in the early boot to cover the entire kernel
	 */
	ASSERT(_kernel_end_phys <= MEMORY_LIMIT,
         "Error: kernel image size exceeds the physical limit! Add extra page tables in the early virtual memory initialization to increase the limit!")
}

