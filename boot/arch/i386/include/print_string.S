
# function that prints the string taken from the stack
print_string:

	push %bp				# save base pointer
	mov %sp, %bp			# set base pointer to stack pointer

	push %si				# save SI register
	push %ax				# save AX register

	mov $0x0e, %ah			# teletype output
	mov 4(%bp), %si			# set SI to the first argument

print_string_loop:
	lodsb					# load the byte in SI into AL and increment SI
	cmp $0x00, %al			# check if it is the null terminator
	je print_string_end		# if it is, end the loop

	int $0x10				# print the character in AL with BIOS interrupt
	jmp print_string_loop	# loop back to the start

print_string_end:

	pop %ax					# restore AX register
	pop %si					# restore SI register

	mov %bp, %sp			# restore stack pointer
	pop %bp					# restore base pointer

	ret						# return from function
