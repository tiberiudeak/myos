
.section .text
.global _start2
.code16

_start2:

	push $msg2				# push the message to the stack
	call print_string		# call the print_string function
	add $0x2, %sp			# remove the pushed value from the stack

	call move_cursor_nl		# call the move_cursor_nl function

	# load inode bitmap into memory (0x9C00 - 0x.... depending on the size)
	pop %si					# pop drive number from stack to SI
	mov (0x8C08), %ax		# first inode bitmap block
	mov $8, %bx				# put 8 into BX to get the sector number
	mul %bx					# AX = AX * 8 (sector number)
	mov %ax, %dx			# put result into DX
	inc %dx					# sector count starts from 1 so add 1
	push %dx				# push starting sector to the stack

	push $0x9C00			# push load address to the stack

	mov (0x8C04), %ax		# number of blocks for the inode bitmap
	mov $8, %bx				# each block has 8 sectors
	mul %bx					# AX = AX * 8 (number of sectors)
	push %ax				# push number of sectors to stack

	push %si				# push drive number to stack

	call disk_load			# call disk load function
	add $0x8, %sp			# restore stack pointer
	push %si				# push drive number back to stack
    call move_cursor_nl

	# at this point, AX will have the number of sectors for the inode bitmap
	mov %ax, %cx			# save that value in CX that will be used later to
							# calculate the starting address for the data bitmap

	# load data bitmap into memory
	pop %si					# pop drive number from stack
	mov (0x8C0A), %ax		# first data bitmap block
	mov $8, %bx				# put 8 into BX to get the sector number
	mul %bx					# AX = AX * 8 (sector number)
	mov %ax, %dx			# put result into DX
	inc %dx					# sector count starts from 1 so add 1
	push %dx				# push starting sector to the stack

	# calculate starting address for the data bitmap
	mov $512, %ax			# move size of a sector into AX
	mul %cx					# multiply that size with the number of sectors
							# occupied by the inode bitmap
	add $0x9C00, %ax		# add total size of the inode bitmap to the
							# starting address of the inode bitmap in order
							# to get the starting address for the data bitmap
	push %ax				# push the resulted address to the stack
	mov %ax, %di			# save starting address in DI

	mov (0x8C06), %ax		# number of blocks for the inode bitmap
	mov $8, %bx				# each block has 8 sectors
	mul %bx					# AX = AX * 8 (number of sectors)
	push %ax				# push number of sectors to stack

	push %si				# push drive number to stack

	call disk_load			# call disk load function
	add $0x8, %sp			# restore stack pointer
	push %si				# push drive number back to stack

	# at this point, AX will have the number of sectors for the data bitmap
	# and DI will have the starting address for the data bitmap blocks
	# information that will be used in the third stage of the bootloader
	# to load the inodes and finally the kernel data

	# detect memory
	call move_cursor_nl
    push $det_mem_msg
    call print_string
    add $0x2, %sp

	push $0x1000			# push 0x1000 to the stack (memory loc.)
	call detect_memory		# call the detect_memory function
	add $0x2, %sp			# remove the pushed value from the stack

    push $done_msg
    call print_string
    add $0x2, %sp

	call move_cursor_nl

    # VESA BIOS Extension
.IFDEF CONFIG_TTY_VBE
    call vbe_get_info       # call vbe get info function

    cmp $0x1, %ax           # see if call was successful
    je .vbe_unsupported     # if not, print error message
.ENDIF

	jmp 0x8600				# jump to third stage bootloader

.include "./include/print_string.S"
.include "./include/print_hex.S"
.include "./include/cursor_next_line.S"
.include "./include/disk_load.S"
.include "./include/detect_memory.S"

.IFDEF CONFIG_TTY_VBE
.include "./include/vbe.S"

.vbe_unsupported:
    push $.vbe_unsupported_msg
    call print_string
    add $0x2, %sp
    cli
    hlt

.vbe_unsupported_msg:
    .asciz "VESA BIOS Extension: unsupported resolution!"
.ENDIF

msg2:
	.asciz "[INFO] Entered 2nd stage boot"

det_mem_msg:
    .asciz "[INFO] Detecting memory using BIOS E820..."

done_msg:
    .asciz "done!"

.space 2048 - (. - _start2)		# fill the rest of the bytes with 0

