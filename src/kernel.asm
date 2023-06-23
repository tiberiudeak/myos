
mov bx, message
call print_string

jmp $

%include "./print/print_string.asm"
%include "./print/print_hex.asm"

message:
	db "Booting MyOS...", 0