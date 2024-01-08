
.section .text
.global _start
.code16

_start:

	# set up stack
	mov $0x9000, %bp			# set base pointer to 0x9000 (free memory)
	mov %bp, %sp				# set stack pointer to base pointer

	# set up segments
	xor %ax, %ax				# clear ax
	mov %ax, %ds				# set data segment to 0
	mov %ax, %es				# set extra segment to 0
	mov %ax, %fs				# set file segment to 0
	mov %ax, %gs				# set general segment to 0
	mov %ax, %ss				# set stack segment to 0

	# save drive number determined by BIOS to stack
	push %dx                    # push dx to stack

	call reset_screen			# call reset_screen

	# print message
	push $msg					# push message address to stack
	call print_string			# call print_string
	add $0x02, %sp				# add 2 to stack pointer

	# move cursor to next line
	mov $0x02, %ah				# set function to 2 (set cursor position)
	mov $0x00, %bh				# set page to 0
	mov $0x01, %dh				# set row to 1
	mov $0x00, %dl				# set column to 0
	int $0x10					# call BIOS interrupt

	# enable A20
	call enable_a20				# call enable_a20

	# make the switch to protected mode
	cli							# clear interrupts
	lgdt gdt_descriptor			# load GDT descriptor

	mov %cr0, %eax				# load cr0 into eax
	or $0x1, %eax				# set bit 0 to 1
	mov %eax, %cr0				# load eax into cr0

	ljmp $CODE_SEG, $protected_mode_init		# far jump to protected mode

	hlt							# halt the CPU

.include "./include/print_string.S"
.include "./include/reset_screen.S"
.include "./include/print_hex.S"
.include "./include/gdt.S"
.include "./include/a20.S"

msg:
	.asciz "Booting MyOS...\n"	# the message

.code32
protected_mode_init:
	mov $DATA_SEG, %ax			# load data segment into ax
	mov %ax, %ds				# load ax into data segment
	mov %ax, %es				# load ax into extra segment
	mov %ax, %fs				# load ax into file segment
	mov %ax, %gs				# load ax into general segment
	mov %ax, %ss				# load ax into stack segment

	mov $0x90000, %ebp			# set base pointer to 0x90000 (free memory)
	mov %ebp, %esp				# set stack pointer to base pointer

	mov $msg2, %si				# load message address into SI
	call print_string_pm		# call print_string_pm

	hlt							# halt the CPU

msg2:
	.asciz "Entered protected mode!"		# the message

.include "./include/print_string_pm.S"

.space 510 - (. - _start)		# fill the rest of the bytes with 0
.word 0xaa55					# boot signature
