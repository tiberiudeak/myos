SECTIONS
{
	.text 0x8000 :
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
