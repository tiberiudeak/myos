
# function that will load sectors from drive to location in memory
# the number of sectors, the drive number, the location and the
# sector number to start reading from are passed as parameters
# disk_load(drive, sectors, location, sector_number)
disk_load:
	push %bp				# save base pointer on stack
	mov %sp, %bp			# set base pointer to stack pointer

	push %ax				# save ax register on stack
	push %cx				# save cx register on stack
	push %dx				# save dx register on stack

	movb $0x02, %ah			# BIOS read sectors from drive function
	movb 6(%bp), %al		# number of sectors to read
	movb $0x00, %ch			# cylinder number 0
	movb 10(%bp), %cl		# sector number
	movb $0x00, %dh			# head number 0
	movb 4(%bp), %dl		# drive number

	xor %bx, %bx			# set bx to 0
	mov %bx, %es			# set es to 0
	mov 8(%bp), %bx			# get memory location to load to

	int $0x13				# call BIOS interrupt 0x13

	jc disk_load_error		# if carry flag is set, error occurred

	pop %dx						# restore dx register
	pop %cx						# restore cx register
	pop %ax						# restore ax register

	mov %bp, %sp				# restore stack pointer
	pop %bp						# restore base pointer
	ret

disk_load_error:
	push disk_error_message		# push error message on stack
	call print_string			# call print_string function
	add $2, %sp					# remove error message from stack

	hlt

disk_error_message:
	.asciz "Disk read error occured!\n"
