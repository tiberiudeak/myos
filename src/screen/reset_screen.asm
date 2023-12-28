;; functions for resetting the screen
reset_screen:
	pusha			; save registers

	mov ah, 0x00	; set video mode
	mov al, 0x03	; 80x25 text mode
	int 0x10		; call BIOS video interrupt

	popa			; restore registers
	ret