
; load DH sectors to ES:BX from drive DL
disk_load:
	push dx
	mov ah, 0x02		; BIOS read sector function
	mov al, dh			; read dh sectors
	mov ch, 0x00		; select cylinder 0
	mov dh, 0x00		; select head 0
	mov cl, 0x02		; start reading from the second sector (after the boot sector)

	int 0x13			; BIOS interrupts for disk functions

	jc disk_error_f		; jump if error (carry flag set)

	pop dx				; restore DX from the stack
	cmp dh, al			; compare actually read sectors with the amount we wanted to
	jne disk_error
	ret

disk_error:
	mov bx, disk_error_message
	call print_string
	jmp $

disk_error_f:
	mov bx, disk_error_message_f
	call print_string
	jmp $

disk_error_message:
	db "Disk read error!",0

disk_error_message_f:
	db "Disk error read in first!",0