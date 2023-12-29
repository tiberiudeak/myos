org 0x8000

main_menu:
	;; reset screen state
	call reset_screen

	;; print menu message to screen
	mov bx, message
	call print_string

	;; ===============================================================
	;; Detecting Low Memory
	;; ===============================================================
	mov bx, low_mem			; print message
	call print_string

	clc						; clear carry flag
	int 0x12				; switch to the bios
	jc end_program			; end execution if error occured

	mov dx, ax				; save number of KiB in DX
	inc dx					; increment size of low momery as it begins from 0
	call print_hex			; print size as hex number

	mov bx, newline
	call print_string

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

	cmp al, 'P'				; print register values
	je register_print

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
	call reset_screen

	;; print header
	mov bx, header
	call print_string

	;; load file table string from its memory location (0x7e00)
	mov bx, 0x7e00
	call print_filetable

	mov bx, file_table_message
	call print_string

get_filename:
	mov di, cmd_string			; cmd_string will store the command
	mov byte [cmd_length], 0	; reset cmd_length

filename_loop:
	xor ah, ah					; set AX to 0
	int 0x16					; BIOS int get keystroke, character goes into al

	mov ah, 0x0e				; BIOS int 1-h teletype output
	mov bh, 0x00

	cmp al, 0xD					; did user press 'enter'
	je search_file
	int 0x10					; BIOS int to print character in AL
	mov [di], al				; store value from DI in AL
	add di, 1					; increment DI
	inc byte [cmd_length]		; increment cmd_length
	jmp filename_loop




search_file:
	;; search for the filename in the file table
	;; filetable is stored in memory location 0x7e00
	;; and has the format: {filename-sector,filename-sector,...}

	mov bx, 0x7e00				; load filetable into SI
	mov si, 0
	mov di, cmd_string			; load filename into DI


search_loop:
	mov al, [bx+si]				; load byte from filetable into AL
	cmp al, '}'
	je file_not_found

	; mov al, si				; load byte from filetable into AL
	cmp al, [di]				; compare byte from filetable with byte from filename

	je match					; if equal, jump to match
	add si, 1					; if not equal, increment SI
	jmp search_loop				; and loop

match:
	mov cl, [cmd_length]		; load cmd_length into CL

	; mov dx, cx
	; call print_hex				; print cmd_length as hex number

	dec cl						; decrement CL

match_loop:

	inc di						; increment DI
	inc si						; increment SI

	mov al, [bx+si]				; load byte from filetable into AL
	cmp byte al, [di]				; compare byte from filetable with byte from filename

	jne restart_search

	dec cl						; decrement CL
	cmp cl, 0					; check if CL is 0
	je file_found				; if CL is 0, jump to file_found

	jmp match_loop				; else, loop

restart_search:

	mov di, cmd_string			; load filename into DI
	jmp search_loop				; restart search

file_found:

	inc si						; increment SI (skip "-" character)
	inc si
	mov bx, filename_found
	call print_string

	mov bx, 0x7e00				; load filetable into BX

	mov cl, 10					; load 10 into CL
	xor al, al					; set AL to 0
	xor dx, dx

sector_number_loop:
	mov dl, [bx+si]				; load byte from filetable into DL

	inc si						; increment SI
	cmp dl, ','					; end of sector number
	je load_file				; if so, jump to load_file
	cmp dl, '}'					; end of filetable
	je load_file				; if so, jump to load_file
	cmp dl, 48					; check if DL is less than 0
	jl sector_not_found			; if so, jump to sector_not_found
	cmp dl, 57					; check if DL is greater than 9
	jg sector_not_found			; if so, jump to sector_not_found
	sub dl, 48					; convert from ASCII to decimal
	mul cl						; multiply AL by CL
	add al, dl					; add DL to AL
	jmp sector_number_loop		; loop

sector_not_found:
	mov bx, sect_not_found
	call print_string
	mov ah, 0x00	; get user keystroke
	int 0x16
	jmp main_menu

load_file:
	clc
	mov cl, al					; load sector number into CL

	; mov ah, 0x00				; set up for int 0x13
	; mov dl, 0x00				; drive number
	; int 0x13					; reset disk



	mov ah, 0x02				; set up for int 0x13
	mov al, 0x01				; number of sectors to read
	mov ch, 0x00				; cylinder number
	mov dh, 0x00				; head number
	mov cl, 0x06					; sector number
	mov dl, 0x80

	xor bx, bx					; set BX to 0
	mov es, bx					; load segment address into ES
	mov bx, 0x9000				; load offset address into BX

	int 0x13					; read sector
	jnc load_success			; if no error, jump to load_success


	mov bx, program_load_fail
	call print_string
	mov ah, 0x00	; get user keystroke
	int 0x16
	jmp main_menu

load_success:
	xor bx, bx					; set BX to 0
	mov es, bx					; load segment address into ES
	mov bx, 0x9000			; load segment address into AX
	; mov ds, ax					; load segment address into DS
	; mov es, ax					; load segment address into ES
	; mov ss, ax					; load segment address into SS
	; mov fs, ax					; load segment address into FS
	; mov gs, ax					; load segment address into GS
	jmp 0x9000			; jump to loaded program

file_not_found:
	mov bx, filename_not_found
	call print_string

temp_end:

	mov bx, go_back
	call print_string

	mov ah, 0x00	; get user keystroke
	int 0x16

	jmp main_menu

;; ===============================================================
;; Print register values
;; ===============================================================
register_print:
	;; reset screen state
	call reset_screen

	;; print header
	mov bx, pr_reg_header
	call print_string

	;; print register values
	call print_registers

	mov bx, go_back
	call print_string

	mov ah, 0x00	; get user keystroke
	int 0x16

	jmp main_menu


%include "./print/print_string.asm"
%include "./print/print_hex.asm"
%include "./print/print_filetable.asm"
%include "./print/print_registers.asm"
%include "./screen/reset_screen.asm"

message:
	db "Booting MyOS...", 0xA, 0xD, 0xA, 0xD, \
	"F) File & Program Browser/Loader", 0xA, 0xD, \
	"R) Reboot", 0xA, 0xD, \
	"P) Print Register Values", 0xA, 0xD, 0

success:
	db " Command successfully executed", 0xA, 0xD, 0

failure:
	db " Oops! Something went wrong :(", 0xA, 0xD, 0

header:
	db "File name            Sector", 0xA, 0xD, "---------            ------", 0xA, 0xD, 0

low_mem:
	db 0xA, "Low Memory Size: ", 0

go_back:
	db 0xA, 0xA, 0xD, "Print any key to go back...", 0

newline:
	db 0xA, 0xD, 0

pr_reg_header:
	db "Register       Value", 0xA, 0xD, \
	"--------       -----", 0xA, 0xD, 0
file_table_message:
	db 0xA, 0xD, "Enter program to load: ", 0

cmd_length:
	db 0

filename_found:
	db "Filename found", 0xA, 0xD, 0

filename_not_found:
	db "Filename not found", 0xA, 0xD, 0

sect_not_found:
	db "Sector not found", 0xA, 0xD, 0

program_load_fail:
	db "Program load failed", 0xA, 0xD, 0

cmd_string:
	db ''

times 1536-($-$$) db 0