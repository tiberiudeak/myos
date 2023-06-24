org 0x7f00

;; print message to screen
mov bx, message
call print_string


;; get user input
get_input:
	mov di, cmd_string		; cmd_string will store the command

keyloop:
	xor ah, ah		; set AX to 0
	int 0x16		; BIOS int get keystroke, character goes into al

	mov ah, 0x0e
	mov bh, 0x00

	cmp al, 0xD		; did user press 'enter'
	je run_command
	int 0x10		; BIOS int to print character in AL
	mov [di], al	; store value from DI in AL
	add di, 1		; increment DI
	jmp keyloop

run_command:
	mov byte [di], 0		; terminate the string with 0
	mov al, [cmd_string]	; store the first byte in cmd_string in AL
	cmp al, 'F'				; check if the command is 'F'

	je cmd_success

	cmp al, 'N'			; check if the command is 'N'
	je end_program		; if so, end the program (halt)

	mov bx, failure		; print failure message
	call print_string
	jmp get_input		; wait for other input

cmd_success:
	mov bx, success		; print success message
	call print_string
	jmp get_input		; wait for other input

end_program:
	jmp $				; loop indefinitely

%include "./print/print_string.asm"
%include "./print/print_hex.asm"

message:
	db "Booting MyOS...", 0xA, 0xD, 0xA, 0xD, "F) File & Program Browser/Loader", 0xA, 0xD, 0

success:
	db " Command successfully executed", 0xA, 0xD, 0

failure:
	db " Oops! Something went wrong :(", 0xA, 0xD, 0

cmd_string:
	db ''

times 512-($-$$) db 0