.extern isr_handler
.extern irq_handler

isr_common_stub:
	pusha
	mov %ds, %eax
	push %eax
	mov %cr2, %eax
	push %eax

	mov $0x10, %ax			# use kernel data segment
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp				# give pointer to stack as parameter to the C function
							# to access the pushed information
	call isr_handler
	add $8, %esp
	pop %ebx

	mov %bx, %ds			# restore old segment
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs

	popa					# restore registers
	add $8, %esp			# remove the error code and interrupt number
	iret					# return from interrupt

irq_common_stub:
	pusha
	mov %ds, %eax
	push %eax
	mov %cr2, %eax
	push %eax

	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	push %esp
	call irq_handler
	add $8, %esp
	pop %ebx

	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	popa
	add $8, %esp
	iret

.globl isr0
.globl isr1
.globl isr2
.globl isr3
.globl isr4
.globl isr5
.globl isr6
.globl isr7
.globl isr8
.globl isr9
.globl isr10
.globl isr11
.globl isr12
.globl isr13
.globl isr14
.globl isr15
.globl isr16
.globl isr17
.globl isr18
.globl isr19
.globl isr20
.globl isr21
.globl isr22
.globl isr23
.globl isr24
.globl isr25
.globl isr26
.globl isr27
.globl isr28
.globl isr29
.globl isr30
.globl isr31
.globl isr128
.globl isr177

isr0:
	push $0
	push $0
	jmp isr_common_stub

isr1:
	push $0
	push $1
	jmp isr_common_stub

isr2:
	push $0
	push $2
	jmp isr_common_stub

isr3:
	push $0
	push $3
	jmp isr_common_stub

isr4:
	push $0
	push $4
	jmp isr_common_stub

isr5:
	push $0
	push $5
	jmp isr_common_stub

isr6:
	push $0
	push $6
	jmp isr_common_stub

isr7:
	push $0
	push $7
	jmp isr_common_stub

isr8:
	push $8
	jmp isr_common_stub

isr9:
	push $0
	push $9
	jmp isr_common_stub

isr10:
	push $10
	jmp isr_common_stub

isr11:
	push $11
	jmp isr_common_stub

isr12:
	push $12
	jmp isr_common_stub

isr13:
	push $13
	jmp isr_common_stub

isr14:
	push $14
	jmp isr_common_stub

isr15:
	push $0
	push $15
	jmp isr_common_stub

isr16:
	push $0
	push $16
	jmp isr_common_stub

isr17:
	push $0
	push $17
	jmp isr_common_stub

isr18:
	push $0
	push $18
	jmp isr_common_stub

isr19:
	push $0
	push $19
	jmp isr_common_stub

isr20:
	push $0
	push $20
	jmp isr_common_stub

isr21:
	push $0
	push $21
	jmp isr_common_stub

isr22:
	push $0
	push $22
	jmp isr_common_stub

isr23:
	push $0
	push $23
	jmp isr_common_stub

isr24:
	push $0
	push $24
	jmp isr_common_stub

isr25:
	push $0
	push $25
	jmp isr_common_stub

isr26:
	push $0
	push $26
	jmp isr_common_stub

isr27:
	push $0
	push $27
	jmp isr_common_stub

isr28:
	push $0
	push $28
	jmp isr_common_stub

isr29:
	push $0
	push $29
	jmp isr_common_stub

isr30:
	push $0
	push $30
	jmp isr_common_stub

isr31:
	push $0
	push $31
	jmp isr_common_stub

isr128:
	push $0
	push $128
	jmp isr_common_stub

isr177:
	push $0
	push $177
	jmp isr_common_stub

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
	push $0
	push $32
	jmp irq_common_stub

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
