
; function that prints the string in BX
print_string:
	pusha					; save registers to stack
	mov ah, 0x0e			; teletype output
	mov si, 0				; index used to go thorough the message in BX

loop:
	mov al, [bx + si]
	cmp al, 0				; if end of string then stop
	je end
	int 0x10
	add si, 1
	jmp loop

end:
	popa					; restore registers from stack
	ret