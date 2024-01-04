;; function to print registers
print_registers:
	mov bx, reg_string
	call print_string
	call print_hex

	mov byte [reg_string+2], 'a'	; change 'dx' to 'ax'
	call print_string
	mov dx, ax
	call print_hex					; print AX

	mov byte [reg_string+2], 'b'	; change 'ax' to 'bx'
	call print_string
	mov dx, bx
	call print_hex					; print BX

	mov byte [reg_string+2], 'c'	; change 'bx' to 'cx'
	call print_string
	mov dx, cx
	call print_hex					; print CX

	mov byte [reg_string+2], 's'	; change 'cx' to 'si'
	mov byte [reg_string+3], 'i'
	call print_string
	mov dx, si
	call print_hex					; print SI

	mov byte [reg_string+2], 'd'	; change 'si' to 'di'
	call print_string
	mov dx, di
	call print_hex					; print DI

	mov byte [reg_string+2], 'c'	; change 'di' to 'cs'
	mov byte [reg_string+3], 's'
	call print_string
	mov dx, cs
	call print_hex					; print CS

	mov byte [reg_string+2], 'd'	; change 'cs' to 'ds'
	call print_string
	mov dx, ds
	call print_hex					; print DS

	mov byte [reg_string+2], 'e'	; change 'ds' to 'es'
	call print_string
	mov dx, es
	call print_hex					; print ES

	;; restore reg_string
	mov byte [reg_string+2], 'd'
	mov byte [reg_string+3], 'x'
	ret

reg_string:
	db 0xA, 0xD, "dx             ", 0		; print DX first