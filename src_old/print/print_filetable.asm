
; function that prints the string in BX
print_filetable:
	mov ah, 0x0e			; teletype output
	mov si, 0				; index used to go thorough the message in BX
	mov cx, 0				; index used to format the output to look good

loop_filetable:
	add si, 1				; ignore first character '{' and increment index
	add cx, 1
	mov al, [bx + si]
	cmp al, '}'				; check if end of file table is reached
	je end_filetable

	cmp al, '-'				; check if end of file name is reached
	je add_whitespaces

	cmp al, 0				; if end of string then stop
	je end_filetable

	int 0x10				; print character to screen
	; add si, 1				; increment index
	jmp loop_filetable

add_whitespaces:
	cmp cx, 22
	je begin_sector
	mov al, ' '
	int 0x10
	add cx, 1
	jmp add_whitespaces

begin_sector:
	mov cx, 0
	add si, 1				; ignore '-' cahracter
loop_sector:
	mov al, [bx + si]
	cmp al, ','				; check if end of sector is reached
	je end_sector

	cmp al, '}'				; check if end of file table is reached
	je end_filetable

	int 0x10				; print character to screen
	add si, 1				; increment index
	jmp loop_sector

;; print '\n' and continue to loop through the file table
end_sector:
	mov al, 0xA
	int 0x10
	mov al, 0xD
	int 0x10
	jmp loop_filetable

end_filetable:
	ret

