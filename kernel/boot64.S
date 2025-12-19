.section .text
.code64
.global long_mode_entry
long_mode_entry:
	mov $0x0f500f50, (0xb8000)
	hlt
	jmp long_mode_entry
