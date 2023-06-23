org 0x7c00

mov ah, 0x00	; set video mode
mov al, 0x03	; 80x25 text mode
int 0x10

xor ax, ax		; set DS to 0 (data segment)
mov ds, ax
cld

mov ah, 2h		; int 13h function 2
mov al, 1		; we want to read 1 sector
mov ch, 0		; cylinder 0
mov cl, 2		; start with the second sector
mov dh, 0		; head number 0

xor bx, bx
mov es, bx		; set ES to 0
mov bx, 7e00h	; and BX to 0x7e00 (EX:BX = 0x7e00)
int 13h			; BIOS interrupt for disk functions

jc disk_error	; if carry flag is set, there was an error

jmp 0x7e00		; jump to second sector

disk_error:
	mov bx, disk_error_message	; load error message into BX
	call print_string			; call print_string function
	jmp $						; loop indefinitely

disk_error_message:
	db "Disk read error!",0

;%include "./print/print_string.asm"

times 510-($-$$) db 0
dw 0xaa55





; SECOND SECTOR - kernel
mov bx, message		; load message into BX
call print_string	; call print_string function

jmp $		; loop indefinitely


%include "./print/print_hex.asm"
%include "./print/print_string.asm"

message db "Booting MyOS...",0