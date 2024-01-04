org 0x7c00

;; ===============================================================
;; Screen setup
;; ===============================================================
mov ah, 0x00	; set video mode
mov al, 0x03	; 80x25 text mode
int 0x10

xor ax, ax		; set DS to 0 (data segment)
mov ds, ax
cld


;; ===============================================================
;; READ FILE TABLE INTO MEMORY (0x7e00)
;; ===============================================================
mov ah, 0x02	; int 13h function 2
mov al, 0x01	; read 1 sector
mov ch, 0x0		; cylinder 0
mov cl, 0x02	; start with the second sector
mov dh, 0x0		; head 0

xor bx, bx		; set BX to 0
mov es, bx		; set ES to 0
mov bx, 0x7e00	; set BX to 0x7e00 (ES:BX = 0x7e00)
int 0x13		; BIOS interrupt for disk functions

;; ===============================================================
;; READ KERNEL INTO MEMORY (0x7f00)
;; ===============================================================
mov ah, 0x02	; int 13h function 2
mov al, 0x03	; we want to read 3 sectors
mov ch, 0x0		; cylinder 0
mov cl, 0x03	; start with the third sector
mov dh, 0x0		; head number 0

xor bx, bx		; set BX to 0
mov es, bx		; set ES to 0
mov bx, 0x8000	; and BX to 0x7f00 (EX:BX = 0x7f00)
int 0x13		; BIOS interrupt for disk functions

jc disk_error	; if carry flag is set, there was an error
; mov ah, 0x02				; set up for int 0x13
; 	mov al, 0x01				; number of sectors to read
; 	mov ch, 0x00				; cylinder number
; 	mov dh, 0x00				; head number
; 	mov cl, 0x06					; sector number

; 	xor bx, bx					; set BX to 0
; 	mov es, bx					; load segment address into ES
; 	mov bx, 0x5000				; load offset address into BX

; 	int 0x13
; jc disk_error	; if carry flag is set, there was an error
jmp 0x8000		; jump to kernel sector

disk_error:
	mov bx, disk_error_message	; load error message into BX
	call print_string			; call print_string function
	jmp $						; loop indefinitely

disk_error_message:
	db "Disk read error!",0

%include "./print/print_string.asm"
%include "./print/print_hex.asm"

times 510-($-$$) db 0
dw 0xaa55