
# Global Descriptor Table Definition
# ==================================
gdt:
gdt_nulldesc:
	.word 0x0000
	.word 0x0000
	.byte 0x00
	.word 0x0000
	.byte 0x00
gdt_codesegment:
	.word 0xffff
	.word 0x0000
	.byte 0x00
	.byte 0b10011010
	.byte 0b11001111
	.byte 0x00
gdt_datasegment:
	.word 0xffff
	.word 0x0000
	.byte 0x00
	.byte 0b10010010
	.byte 0b11001111
	.byte 0x00
gdt_end:

gdt_descriptor:
	.word gdt_end - gdt - 1
	.long gdt

DATA_SEG = gdt_datasegment - gdt
CODE_SEG = gdt_codesegment - gdt
