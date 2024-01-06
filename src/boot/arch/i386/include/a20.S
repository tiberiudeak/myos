
# function that activates the A20 line using BIOS interrupt 0x15
enable_a20:
	pusha					# save registers

	mov $0x2403, %ax		# function 0x2403
	int $0x15				# call BIOS
	jb a20_ns				# jump if carry flag is set
	cmp $0, %ah				# check if function is supported
	jnz a20_ns				# jump if not supported

	mov $0x2402, %ax		# function 0x2402
	int $0x15				# call BIOS
	jb a20_failed			# jump if carry flag is set
	cmp $0, %ah				# check if function failed
	jnz a20_failed			# jump if failed

	cmp $1, %al				# check if A20 is enabled
	jz a20_ok				# jump if enabled

	mov $0x2401, %ax		# function 0x2401
	int $0x15				# call BIOS
	jb a20_failed			# jump if carry flag is set
	cmp $0, %ah				# check if function failed
	jnz a20_failed			# jump if failed

a20_ns:
	mov $a20_ns_msg, %si	# load message
	call print_string		# print message
	popa					# restore registers
	ret						# return

a20_failed:
	mov $a20_failed_msg, %si	# load message
	call print_string			# print message
	popa						# restore registers
	ret							# return

a20_ok:
	mov $a20_ok_msg, %si	# load message
	call print_string		# print message
	popa					# restore registers
	ret						# return

a20_ns_msg:
	.asciz "A20 not supported\n"

a20_failed_msg:
	.asciz "A20 failed to enable\n"

a20_ok_msg:
	.asciz "A20 enabled\n"
