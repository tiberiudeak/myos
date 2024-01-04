[bits 32]
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

; print string stored in EBX
print_string_pm:
	pusha
	mov edx, VIDEO_MEMORY			; set EDX to the start of the video memory

print_string_pm_loop:
	mov al, [ebx]					; store the byte pointed to by EBX in AL
	mov ah, WHITE_ON_BLACK			; store the attribute byte in AH

	cmp al, 0						; check if end of string reached
	je done

	mov [edx], ax					; store character and attribute in video memory

	add ebx, 1						; increment EBX to point to next character
	add edx, 2						; move to next video memory cell

	jmp print_string_pm_loop		; loop until end of string

done:
	popa
	ret
