.extern isr_handler
.extern isr_prob
.extern current_running_task

isr_common_stub:
	cli
	pusha					# push current state of registers to stack
	mov %ds, %eax
	push %eax				# push DS to stack
	mov %cr2, %eax
	push %eax				# push CR2 to stack

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

	mov %bx, %ds			# restore old segments
	mov %bx, %es
	mov %bx, %fs
	mov %bx, %gs

	popa					# restore registers

	# check if isr was syscall
	cmpl $0x80, (%esp)
	jne isr_common_stub_pre_exit
	add $8, %esp			# remove the error code and interrupt number

	cmpl $0x2, (isr_prob)			# manual pop problem
	jne	isr_common_stub_exit
	push %eax					# save EAX
	mov 0xc(%esp), %eax			# get EFLAGS
	mov %eax, 0x14(%esp)		# and put it in the place of SS
	mov 0x8(%esp), %eax			# get CS
	mov %eax, 0x10(%esp)		# and put it in the place of ESP
	mov 0x4(%esp), %eax			# get EIP
	mov %eax, 0xc(%esp)			# and put it in the place of initial EFLAGS
	pop %eax					# restore EAX
	add $0x8, %esp				# delete initial (now moved) CS and EIP
	mov $0x0, (isr_prob)		# reset prob
	jmp isr_common_stub_exit	# and exit

isr_common_stub_pre_exit:
	add $8, %esp			# remove the error code and interrupt number
isr_common_stub_exit:
	iret					# return from interrupt

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

isr0:
	push $0					# push error code
	push $0					# push interrupt number
	jmp isr_common_stub		# call the isr common stub function

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
	push $8					# for the isrs 8, 10, 11, 12, 13 and 14
							# the cpu already pushes the error code
							# so there is no need to push a dummy error code
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
