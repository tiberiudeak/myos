
; function that prints the value of DX as hex
print_hex:
	; manipulate chars at hex_out to reflect dx
	pusha					; save registers to stack
	mov cx, 4				; initialize counter with 4

char_loop:
	dec cx					; decrement counter
	mov ax, dx				; copy dx into ax
	shr dx, 4				; right shift with 4 bits
	and ax, 0xf				; apply mask to ax

	mov bx, hex_out			; store memory address of the template into bx
	add bx, 2				; skip forst two characters ('0x')
	add bx, cx				; go to current position in the template

	cmp ax, 0xa				; check if hex is letter
	jl letter
	add byte [bx], 7		; if hex is letter then add 7

letter:
	add byte [bx], al		; add the byte in al to value of bx
	cmp cx, 0
	je hex_done
	jmp char_loop

hex_done:
	mov bx, hex_out
	call print_string

	; restore template to 0x0000
	mov byte [hex_out+2], '0'
	mov byte [hex_out+3], '0'
	mov byte [hex_out+4], '0'
	mov byte [hex_out+5], '0'

	popa			; restore registers from stack
	ret

hex_out:
	db "0x0000",0