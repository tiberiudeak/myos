SECTIONS
{
	.text 0xDC00 :
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
