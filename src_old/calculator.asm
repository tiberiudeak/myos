org 0x9000

call reset_screen

mov bx, message
call print_string

mov ah, 0x00
int 0x16

mov ax, 0x8000
mov es, ax
xor bx, bx

mov dx, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
jmp 0x8000


%include "./print/print_string.asm"
%include "./screen/reset_screen.asm"

message:
	db "Program loaded successfully", 0

times 512-($-$$) db 0