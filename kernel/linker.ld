ENTRY (_start)

KERNEL_VIRT_ADDR_BASE = 0xC0000000;

SECTIONS
{
	/* load kernel above 1MB */
    . = 0x00100000;

	/* start with the multiboot header, followed by the
	   initial code that initializes the page directory
	   and page table needed to map the higher half kernel
	 */
	_kernel_start = .;
	.multiboot.data : {
		*(.multiboot.data)
	}

   .multiboot.text : {
	   *(.multiboot.text)
   }

	. += KERNEL_VIRT_ADDR_BASE;
	.text ALIGN (4K) : AT (ADDR (.text) - KERNEL_VIRT_ADDR_BASE)
	{
		*(.text)
	}
	.rodata ALIGN (4K) : AT (ADDR (.rodata) - KERNEL_VIRT_ADDR_BASE)
	{
		*(.rodata)
	}
	.data ALIGN (4K) : AT (ADDR (.data) - KERNEL_VIRT_ADDR_BASE)
	{
		*(.data)
	}
	.bss ALIGN (4K) : AT (ADDR (.bss) - KERNEL_VIRT_ADDR_BASE)
	{
		*(COMMON)
		*(.bss)
		*(.bootstrap_stack)
	}
	_kernel_end = .;
}

