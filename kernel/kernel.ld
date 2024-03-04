SECTIONS
{
	.text 0xF000 :
	{
		*(kernel_entry);
		*(.text*);
	}
	.data :
	{
		*(.data*);
	}
	.rodata :
	{
		*(.rodata*);
	}
	.bss :
	{
		*(.bss*);
	}

	PROVIDE(kernel_end = .);
}
