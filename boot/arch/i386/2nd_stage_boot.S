
.section .text
.global _start2
.code16

_start2:

	push $msg2				# push the message to the stack
	call print_string		# call the print_string function
	add $0x2, %sp			# remove the pushed value from the stack

	call move_cursor_nl		# call the move_cursor_nl function

	# load the kernel to 0x8000 (after the bootloader)
	pop %dx					# pop drive number from stack to dx
	push $0x3				# push 3 to the stack (sector number)
	push $0x8000			# push 0x8000 to the stack (load address)
	push $0x20				# push 32 to the stack (sector count)
	push $0x80				# push 0 to the stack (drive number)
	call disk_load			# call the disk_load function
	add $0x8, %sp			# remove the 3 pushed values from the stack
	push %dx				# push the drive number back to the stack


	# detect memory
	push $0x1000			# push 0x1000 to the stack (memory loc.)
	call detect_memory		# call the detect_memory function
	add $0x2, %sp			# remove the pushed value from the stack


	# empty stack
	pop %dx


	# make switch to protected mode
	cli						# disable interrupts
	lgdt gdt_descriptor		# load the gdt descriptor

	mov %cr0, %eax			# move the value of cr0 to eax
	or $0x1, %eax			# set the first bit of eax to 1 (protected mode)
	mov %eax, %cr0			# move the value of eax to cr0

	ljmp $CODE_SEG, $protected_mode_init	# jump to protected mode

.include "./include/print_string.S"
# .include "./include/print_hex.S"
.include "./include/cursor_next_line.S"
.include "./include/disk_load.S"
.include "./include/gdt.S"
.include "./include/detect_memory.S"

msg2:
	.asciz "Entered 2nd stage boot"


.code32
protected_mode_init:
	mov $DATA_SEG, %ax	        # load data segment into ax
	mov %ax, %ds                # load ax into data segment register
	mov %ax, %es                # load ax into extra segment register
	mov %ax, %fs                # load ax into fs segment register
	mov %ax, %gs                # load ax into gs segment register
	mov %ax, %ss                # load ax into stack segment register

	mov $0x90000, %ebp			# set the base pointer to 0x90000
	mov %ebp, %esp				# set the stack pointer to base pointer

	call 0x8000

g:
	jmp g

.space 512 - (. - _start2)		# fill the rest of the bytes with 0
