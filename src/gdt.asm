; GDT definition
gdt:
gdt_nulldesc:
    dd 0x0 ; null descriptor - just padding
	dd 0x0
gdt_codesegment:
	dw 0xffff ; limit low (16 bits)
	dw 0x0000 ; base low (16 bits)
	db 0x00 ; base middle (8 bits)
	db 10011010b ; flags
	db 11001111b ; flags
	db 0x00 ; base high (8 bits)
gdt_datasegment:
	dw 0xffff ; limit low (16 bits)
	dw 0x0000 ; base low (16 bits)
	db 0x00 ; base middle (8 bits)
	db 10010010b ; flags
	db 11001111b ; flags
	db 0x00 ; base high (8 bits)
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt - 1 ; size (16 bits)
	dd gdt ; address (32 bits)

DATA_SEG equ gdt_datasegment - gdt
CODE_SEG equ gdt_codesegment - gdt
