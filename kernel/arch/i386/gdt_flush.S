.globl gdt_flush

gdt_flush:
	mov 4(%esp), %eax
	lgdt (%eax)

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ljmp $0x08, $gdt_flushed

gdt_flushed:
	ret


.globl tss_flush

tss_flush:
	mov $0x2B, %ax
	ltr %ax
	ret
