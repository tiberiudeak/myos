.section .text
.code64
.global long_mode_entry
.extern stack_top
long_mode_entry:
	// set 64bit stack
	mov stack_top, %rax

	// clear segment registers
	xor %rax, %rax
	mov %rax, %ss
	mov %rax, %ds
	mov %rax, %es
	mov %rax, %fs
	mov %rax, %gs

	// reset flags
	push $0
	popf

	movl $0x0f500f50, (0xb8000)

	call kmain

	cli
	// endless loop, should not be reached
loop:
	hlt
	jmp loop
