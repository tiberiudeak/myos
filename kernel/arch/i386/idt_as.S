.extern isr_handler
.extern irq_handler

isr_common_stub:
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
	call isr_handler
	add $8, %esp
	pop %ebx

	mov %bx, %ds
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs

	popa
	add $8, %esp
	sti
	iret

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
	sti
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
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $0, 1(%esp)
	jmp isr_common_stub

isr1:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $1, 1(%esp)
	jmp isr_common_stub

isr2:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $2, 1(%esp)
	jmp isr_common_stub

isr3:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $3, 1(%esp)
	jmp isr_common_stub

isr4:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $4, 1(%esp)
	jmp isr_common_stub

isr5:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $5, 1(%esp)
	jmp isr_common_stub

isr6:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $6, 1(%esp)
	jmp isr_common_stub

isr7:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $7, 1(%esp)
	jmp isr_common_stub

isr8:
	cli
	add $-1, %esp
	movb $8, 0(%esp)
	jmp isr_common_stub

isr9:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $9, 1(%esp)
	jmp isr_common_stub

isr10:
	cli
	add $-1, %esp
	movb $10, 0(%esp)
	jmp isr_common_stub

isr11:
	cli
	add $-1, %esp
	movb $11, 0(%esp)
	jmp isr_common_stub

isr12:
	cli
	add $-1, %esp
	movb $12, 0(%esp)
	jmp isr_common_stub

isr13:
	cli
	add $-1, %esp
	movb $13, 0(%esp)
	jmp isr_common_stub

isr14:
	cli
	add $-1, %esp
	movb $14, 0(%esp)
	jmp isr_common_stub

isr15:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $15, 1(%esp)
	jmp isr_common_stub

isr16:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $16, 1(%esp)
	jmp isr_common_stub

isr17:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $17, 1(%esp)
	jmp isr_common_stub

isr18:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $18, 1(%esp)
	jmp isr_common_stub

isr19:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $19, 1(%esp)
	jmp isr_common_stub

isr20:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $20, 1(%esp)
	jmp isr_common_stub

isr21:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $21, 1(%esp)
	jmp isr_common_stub

isr22:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $22, 1(%esp)
	jmp isr_common_stub

isr23:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $23, 1(%esp)
	jmp isr_common_stub

isr24:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $24, 1(%esp)
	jmp isr_common_stub

isr25:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $25, 1(%esp)
	jmp isr_common_stub

isr26:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $26, 1(%esp)
	jmp isr_common_stub

isr27:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $27, 1(%esp)
	jmp isr_common_stub

isr28:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $28, 1(%esp)
	jmp isr_common_stub

isr29:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $29, 1(%esp)
	jmp isr_common_stub

isr30:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $30, 1(%esp)
	jmp isr_common_stub

isr31:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $31, 1(%esp)
	jmp isr_common_stub

isr128:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $128, 1(%esp)
	jmp isr_common_stub

isr177:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $177, 1(%esp)
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
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $0, 32(%esp)
	jmp irq_common_stub

irq1:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $1, 33(%esp)
	jmp irq_common_stub

irq2:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $2, 34(%esp)
	jmp irq_common_stub

irq3:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $3, 35(%esp)
	jmp irq_common_stub

irq4:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $4, 36(%esp)
	jmp irq_common_stub

irq5:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $5, 37(%esp)
	jmp irq_common_stub

irq6:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $6, 38(%esp)
	jmp irq_common_stub

irq7:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $7, 39(%esp)
	jmp irq_common_stub

irq8:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $8, 40(%esp)
	jmp irq_common_stub

irq9:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $9, 41(%esp)
	jmp irq_common_stub

irq10:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $10, 42(%esp)
	jmp irq_common_stub

irq11:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $11, 43(%esp)
	jmp irq_common_stub

irq12:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $12, 44(%esp)
	jmp irq_common_stub

irq13:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $13, 45(%esp)
	jmp irq_common_stub

irq14:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $14, 46(%esp)
	jmp irq_common_stub

irq15:
	cli
	add $-2, %esp
	movb $0, (%esp)
	movb $15, 47(%esp)
	jmp irq_common_stub
