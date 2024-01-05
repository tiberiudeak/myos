
# function that resets the screen to default
reset_screen:
	pusha				# push all registers to stack

	movb $0x00, %ah		# set function to set video mode
	movb $0x03, %al		# set video mode to 80x25 text mode
	int $0x10			# call BIOS video interrupt

	popa				# pop all registers from stack
	ret					# return to caller
