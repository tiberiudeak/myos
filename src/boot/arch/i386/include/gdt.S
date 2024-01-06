
# Global Descriptor Table Definition
# ==================================

# The GDT is a table of 8-byte entries, each of which describes a segment of
# memory. The first entry is a null descriptor, which is required by the
# processor. In my case, the second entry is a code segment descriptor, and the
# third entry is a data segment descriptor.

# Each entry is 8 bytes long and consists of the following fields:
#  31           24 23      20 19        16 15     12 11    8 7                   0
# |---------------------------------------|---------------------------------------|
# |BaseAddr(24-31)|G|DB|L|AVL|Limit(16-19)|P| DPL |S| Type  |   Base addr (16-23) |
# |---------------------------------------|---------------------------------------|
# |      Base address (Bit 0-15)		  | 	Segment limit (Bit 0-15)	 	  |
# |---------------------------------------|---------------------------------------|

# P: Present bit
# DPL: Descriptor Privilege Level
# S: Descriptor type (0 = system, 1 = code or data)
# Type: Segment type (0 = data, 1 = code)
# AVL: Available for use by system software
# L: Long mode
# DB: Default operation size (0 = 16-bit segment, 1 = 32-bit segment)
# G: Granularity (0 = 1 byte, 1 = 4KB)

gdt:
gdt_nulldesc:
	.word 0x0000
	.word 0x0000
	.byte 0x00
	.word 0x0000
	.byte 0x00
gdt_codesegment:
	.word 0xffff		# Segment limit (Bit 0-15)
	.word 0x0000		# Base address (Bit 0-15)
	.byte 0x00			# Base address (Bit 16-23)
	.byte 0b10011010	# P=1, DPL=0, S=1, Type=0b1010 (code)
	.byte 0b11001111	# G=1, DB=1, L=0, AVL=0, Limit=0b1111
	.byte 0x00			# Base address (Bit 24-31)
gdt_datasegment:
	.word 0xffff		# Segment limit (Bit 0-15)
	.word 0x0000		# Base address (Bit 0-15)
	.byte 0x00			# Base address (Bit 16-23)
	.byte 0b10010010	# P=1, DPL=0, S=1, Type=0b0010 (data)
	.byte 0b11001111	# G=1, DB=1, L=0, AVL=0, Limit=0b1111
	.byte 0x00			# Base address (Bit 24-31)
gdt_end:

# The GDT descriptor is a 6-byte structure that points to the GDT and specifies
# its size. The first two bytes are the size of the GDT minus one, and the next
# four bytes are the address of the GDT.

gdt_descriptor:
	.word gdt_end - gdt - 1
	.long gdt

# Segment Selector Definitions
DATA_SEG = gdt_datasegment - gdt
CODE_SEG = gdt_codesegment - gdt
