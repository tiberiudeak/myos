
# function that will move the cursor from the current position to the
# position of the first character of the next line
move_cursor_nl:

	# save registers (some are changed by BIOS interrupt)
	push %ax					# save register AX
	push %bx					# save register BX
	push %cx					# save register CX
	push %dx					# save register DX

	# get current position using BIOS interrupt 0x10
	xor %ax, %ax				# clear AX
	xor %bx, %bx				# clear BX
	mov $0x03, %ah				# get cursor position function
	mov $0x00, %bh				# page number

	int $0x10					# call BIOS interrupt

	# set cursor to new position
	mov $0x02, %ah				# set cursor position function
	mov $0x00, %bh				# page number
	inc %dh						# increment row number
	movb $0x00, %dl				# column number

	int $0x10					# call BIOS interrupt

	# restore registers
	pop %dx						# restore register DX
	pop %cx						# restore register CX
	pop %bx						# restore register BX
	pop %ax						# restore register AX

	ret							# return from function
