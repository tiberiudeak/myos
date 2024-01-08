
# function that prints the value in DX as hex
print_hex:
	push %bp				# save base pointer on stack
	mov %sp, %bp			# set base pointer to stack pointer

	push %cx				# save CX on stack
	push %dx				# save DX on stack
	push %bx				# save BX on stack
	push %ax				# save AX on stack

	mov $4, %cx				# set counter to 4
	mov 4(%bp), %dx			# move value to DX

char_loop:
	dec %cx					# decrement counter
	mov %dx, %ax			# move value in DX to AX
	shr $4, %dx				# shift DX right by 4 bits
	and $0x000F, %ax		# mask AX with 0x000F

	mov $hex_out, %bx		# move address of hex_out to BX
	add $2, %bx				# add 2 to BX to skip the '0x' part
	add %cx, %bx			# add counter to BX to get the correct position

	cmp $0xA, %ax			# check if hex is letter
	jl letter
	addb $7, (%bx)			# if so, add 7 to get correct letter

letter:
	addb %al, (%bx)			# add value in AX to hex_out
	cmp $0, %cx				# check if counter is 0
	je hex_done				# if so, we are done
	jmp char_loop			# else, loop again

hex_done:
	push $hex_out			# push address of hex_out to stack
	call print_string		# print hex_out
	add $2, %sp				# remove address of hex_out from stack

	# restore template to 0x0000
	mov $hex_out, %bx		# move address of hex_out to SI
	movb $'0', 0x0002(%bx)
	movb $'0', 0x0003(%bx)
	movb $'0', 0x0004(%bx)
	movb $'0', 0x0005(%bx)

	pop %ax					# restore AX from stack
	pop %bx					# restore BX from stack
	pop %dx					# restore DX from stack
	pop %cx					# restore CX from stack
	mov %bp, %sp			# restore stack pointer from base pointer
	pop %bp					# restore base pointer from stack
	ret						# return

hex_out:
	.asciz "0x0000"
