.extern irq_handler

irq_common_stub:
    cli
	pusha						# push current state of registers to stack
	mov %ds, %eax
	push %eax					# push data segment to stack
	mov %cr2, %eax
	push %eax					# push CR2 to stack

	mov $0x10, %ax				# use kernel data segment
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp					# give pointer to stack as parameter to the irq_handler
								# function in order to access the pushed information
	call irq_handler
	add $8, %esp
	pop %eax

	mov %ax, %ds				# restore old segment
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	popa						# restore registers
	add $8, %esp				# remove error code and interrupt number
	iret						# return from interrupt

.globl irq0
.globl irq1
.globl irq2
.globl irq3
.globl irq4
.globl irq5
.globl irq6
.globl irq7
.globl irq8
.globl irq9
.globl irq10
.globl irq11
.globl irq12
.globl irq13
.globl irq14
.globl irq15

irq0:
	push $0						# push error code
	push $32					# push interrupt number
	jmp irq_common_stub			# call the irq common stub function

irq1:
	push $1
	push $33
	jmp irq_common_stub

irq2:
	push $2
	push $34
	jmp irq_common_stub

irq3:
	push $3
	push $35
	jmp irq_common_stub

irq4:
	push $4
	push $36
	jmp irq_common_stub

irq5:
	push $5
	push $37
	jmp irq_common_stub

irq6:
	push $6
	push $38
	jmp irq_common_stub

irq7:
	push $7
	push $39
	jmp irq_common_stub

irq8:
	push $8
	push $40
	jmp irq_common_stub

irq9:
	push $9
	push $41
	jmp irq_common_stub

irq10:
	push $10
	push $42
	jmp irq_common_stub

irq11:
	push $11
	push $43
	jmp irq_common_stub

irq12:
	push $12
	push $44
	jmp irq_common_stub

irq13:
	push $13
	push $45
	jmp irq_common_stub

irq14:
	push $14
	push $46
	jmp irq_common_stub

irq15:
	push $15
	push $47
	jmp irq_common_stub
