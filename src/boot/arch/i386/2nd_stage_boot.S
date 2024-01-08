
.section .text
.global _start
.code16

_start:

	push $msg2
	call print_string

	call move_cursor_nl

t:
	jmp t

.include "./include/print_string.S"
.include "./include/cursor_next_line.S"

msg2:
	.asciz "Entered 2nd stage boot"


.space 512 - (. - _start)		# fill the rest of the bytes with 0
