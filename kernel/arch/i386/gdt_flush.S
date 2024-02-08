.globl gdt_flush

gdt_flush:
	mov 4(%esp), %eax			# store in EAX the function parameter
								# which is the GDT address
	lgdt (%eax)					# load the GDT

	mov $0x10, %ax				# set segments to kernel data segment
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ljmp $0x08, $gdt_flushed	# set code segment to the corresponding entry
								# in the GDT (0x08 is the kernel code entry in the GDT)

gdt_flushed:
	ret

.globl tss_flush

tss_flush:
	mov $0x2B, %ax				# index of the TSS structure is at 0x28
								# as it is the 6th entry in the GDT, but it needs to
								# be set to the bottom two bits of the entry
	ltr %ax						# load the TSS
	ret
