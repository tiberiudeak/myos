
# function that activates the A20 line using BIOS interrupt 0x15
enable_a20:
	push %ax				# save ax register
	push %si				# save si register

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
	push $a20_ns_msg		# push message
	call print_string		# print message
	add $2, %sp				# remove message from stack
	jmp a20_end				# jump to end

a20_failed:
	push $a20_failed_msg		# push message
	call print_string			# print message
	add $2, %sp					# remove message from stack
	jmp a20_end					# jump to end

a20_ok:
	push $a20_ok_msg		# push message
	call print_string		# print message
	add $2, %sp				# remove message from stack

a20_end:

	pop %si					# restore si register
	pop %ax					# restore ax register

	ret						# return

a20_ns_msg:
	.asciz "[ERROR] A20 not supported"

a20_failed_msg:
	.asciz "[ERROR] A20 failed to enable"

a20_ok_msg:
	.asciz "[INFO] A20 enabled"
