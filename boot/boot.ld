SECTIONS
{
	.text 0x7C00 :
	{
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
}
