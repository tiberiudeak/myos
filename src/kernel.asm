org 0x7f00

;; print menu message to screen
mov bx, message
call print_string

;; ===============================================================
;; Detecting Low Memory
;; ===============================================================
mov bx, low_mem				; print message
call print_string

clc							; clear carry flag
int 0x12					; switch to the bios
jc end_program				; end execution if error occured

mov dx, ax					; save number of KiB in DX
inc dx						; increment size of low momery as it begins from 0
call print_hex				; print size as hex number

;; ===============================================================
;; Get user input and execute command
;; ===============================================================
get_input:
	mov di, cmd_string		; cmd_string will store the command

keyloop:
	xor ah, ah				; set AX to 0
	int 0x16				; BIOS int get keystroke, character goes into al

	mov ah, 0x0e			; BIOS int 1-h teletype output
	mov bh, 0x00

	cmp al, 0xD				; did user press 'enter'
	je run_command
	int 0x10				; BIOS int to print character in AL
	mov [di], al			; store value from DI in AL
	add di, 1				; increment DI
	jmp keyloop

run_command:
	mov byte [di], 0		; terminate the string with 0
	mov al, [cmd_string]	; store the first byte in cmd_string in AL
	cmp al, 'F'				; check if the command is 'F'

	je filebrowser

	cmp al, 'N'				; check if the command is 'N'
	je end_program			; if so, end the program (halt)

	cmp al, 'R'				; warm reboot
	je reboot

	mov bx, failure			; print failure message
	call print_string
	jmp get_input			; wait for other input

cmd_success:
	mov bx, success			; print success message
	call print_string
	jmp get_input			; wait for other input

;; ===============================================================
;; End program - jump indefinitely
;; ===============================================================
end_program:
	jmp $					; loop indefinitely

;; ===============================================================
;; Reboot - far jump to reset vector
;; ===============================================================
reboot:
	jmp 0xFFFF:0x0000


;; ===============================================================
;; File browser and loader
;; ===============================================================
filebrowser:
	;; reset screen state
	mov ah, 0x00	; int 0x10 set video mode
	mov al, 0x03	; 80x25 text mode
	int 0x10

	;; print header
	mov bx, header
	call print_string

	;; load file table string from its memory location (0x7e00)
	mov bx, 0x7e00
	call print_filetable

	jmp end_program


%include "./print/print_string.asm"
%include "./print/print_hex.asm"
%include "./print/print_filetable.asm"

message:
	db "Booting MyOS...", 0xA, 0xD, 0xA, 0xD, "F) File & Program Browser/Loader", 0xA, 0xD, "R) Reboot", 0xA, 0xD, 0

success:
	db " Command successfully executed", 0xA, 0xD, 0

failure:
	db " Oops! Something went wrong :(", 0xA, 0xD, 0

header:
	db "File name            Sector", 0xA, 0xD, "---------            ------", 0xA, 0xD, 0

low_mem:
	db 0xA, "Low Memory Size: ", 0

cmd_string:
	db ''

times 512-($-$$) db 0