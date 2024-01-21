
# function that prints the string in SI using the video memory
.set VIDEO_MEMORY, 0xb8000
.set WHITE_ON_BLACK, 0x0f

print_string_pm:
	pusha						# save registers
	mov $VIDEO_MEMORY, %edx		# store the address of the video memory in edx


print_string_pm_loop:
	lodsb						# load the next byte from the string into AL
	mov $WHITE_ON_BLACK, %ah	# store the attribute byte in ah

	cmp $0, %al					# check if AL is zero
	je print_string_pm_done		# if it is, jump to done

	mov %ax, (%edx)				# store AX at the video memory address

	add $2, %edx				# move to the next character cell

	jmp print_string_pm_loop	# repeat until we reach the end of the string

print_string_pm_done:
	popa						# restore registers
	ret							# return from the function
