[org 0x7c00]
[bits 16]
mov bp, 0x9000
mov sp, bp

xor ax, ax
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax

call reset_screen

mov bx, message
call print_string


cli
lgdt [gdt_descriptor]

mov eax, cr0
or eax, 0x1
mov cr0, eax

jmp CODE_SEG:init_protected_mode

%include "print_string.asm"
%include "reset_screen.asm"
%include "gdt.asm"

message: db "Booting MyOS...", 0xA,0xD,0
; message3: db "Disk error", 0xA,0xD,0

[bits 32]

init_protected_mode:
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	mov ebp, 0x90000
	mov esp, ebp

	mov ebx, message2
	call print_string_pm

	jmp $


message2: db "Protected Mode",0
message4: db "Loading kernel into memory...", 0xA,0xD,0

%include "print_string_pm.asm"
times 510-($-$$) db 0
dw 0xaa55