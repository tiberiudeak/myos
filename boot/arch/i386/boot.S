
.section .text
.global _start
.code16

_start:

	# set up stack
	mov $0x7c00, %bp			# set base pointer to 0x7c00; the stack
                                # grows downwards from here, so it will
								# not overwrite the bootloader
	mov %bp, %sp				# set stack pointer to base pointer

	# set up segments
	xor %ax, %ax				# clear ax
	mov %ax, %ds				# set data segment to 0
	mov %ax, %es				# set extra segment to 0
	mov %ax, %fs				# set file segment to 0
	mov %ax, %gs				# set general segment to 0
	mov %ax, %ss				# set stack segment to 0

	# save drive number determined by BIOS to stack
	push %dx					# push dx to stack

	call reset_screen			# call reset_screen

	# print message
	push $msg					# push message address to stack
	call print_string			# call print_string
	add $0x02, %sp				# add 2 to stack pointer

	# move cursor to next line
	call move_cursor_nl			# call cursor_next_line

	# enable A20
	call enable_a20				# call enable_a20

	# move cursor to next line
	call move_cursor_nl			# call cursor_next_line

	# load boot block and superblock at address 0x7E00 (starting with sector 2)
	# boot block: 0x7C00 - 0x8C00
	# superblock: 0x8C00 - 0x9C00
	pop %dx						# pop drive number from stack to dx
	push $0x2					# push 2 to stack (sector number)
	push $0x7e00				# push 0x7e00 to stack (load address)
	push $15 					# push 15 to stack (boot block and superblock)
								# sector 1 is loaded by the BIOS
	push %dx					# push dl to stack (drive number)
	call disk_load				# call disk_load
	add $0x08, %sp				# add 8 to stack pointer
	push %dx					# push drive number back to stack

	jmp 0x7e00					# jump to 2nd stage boot sector


.include "./include/print_string.S"
.include "./include/reset_screen.S"
#.include "./include/print_hex.S"
.include "./include/a20.S"
.include "./include/disk_load.S"
.include "./include/cursor_next_line.S"

msg:
	.asciz "Booting MyOS..."	# the message

.space 510 - (. - _start)		# fill the rest of the bytes with 0
.word 0xaa55					# boot signature
