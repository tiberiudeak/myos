
# function that prints the string in the SI register
print_string:
    pusha					# save registers to stack
	mov $0x0e, %ah			# teletype output

print_string_loop:
	lodsb					# load the byte in SI into AL and increment SI
	cmp $0x00, %al			# check if it is the null terminator
	je print_string_end		# if it is, end the loop

	int $0x10				# print the character in AL with BIOS interrupt
	jmp print_string_loop	# loop back to the start

print_string_end:
	popa					# restore registers from stack
	ret						# return from function
