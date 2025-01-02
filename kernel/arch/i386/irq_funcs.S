.extern irq_handler
.extern irq_prob
.extern current_running_task

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

	# fix potential problems only for timer interrupt
	cmpl $0x20, (%esp)			# check interrupt number
	jne irq_common_stub_pre_exit
	add $8, %esp				# remove error code and interrupt number

irq_manual_push:
	btl $0x0, (irq_prob)		# check bit 1 (MANUAL_PUSH)
	jnc irq_manual_pop
	sub $0x8, %esp				# make place for ESP and SS
	push %eax					# save EAX


	mov 0xc(%esp), %eax			# get EIP from stack
	mov %eax, 0x4(%esp)			# and put it in the new place
	mov 0x10(%esp), %eax		# get CS from stack
	mov %eax, 0x8(%esp)			# and put it in the new place
	mov 0x14(%esp), %eax		# get EFLAGS from stack
	mov %eax, 0xc(%esp)			# and put it in the new place

	mov (current_running_task), %eax
	mov 0x18(%eax), %eax		# get current_running_task's context
	mov 0x40(%eax), %eax		# get USERESP from the context
	mov %eax, 0x10(%esp)		# and save it onto the stack

	mov (current_running_task), %eax
	mov 0x18(%eax), %eax		# get current_running_task's context
	mov 0x3c(%eax), %eax		# get SS from the context
	mov %eax, 0x14(%esp)		# and save it onto the stack

	pop %eax					# restore EAX
	andl $0xfffffffe, (irq_prob)		# reset MANUAL_PUSH bit
	jmp irq_change_kstack		# and check for other problem


irq_manual_pop:
	btl $0x1, (irq_prob)		# check bit 2 (MANUAL_POP)
	jnc	irq_change_kstack
	push %eax					# save EAX

	mov 0xc(%esp), %eax			# get EFLAGS
	mov %eax, 0x14(%esp)		# and put it in place of SS

	mov 0x8(%esp), %eax			# get CS
	mov %eax, 0x10(%esp)		# and put it in place of ESP

	mov 0x4(%esp), %eax			# get EIP
	mov %eax, 0xc(%esp)			# and put it in the place of initial EFLAGS

	pop %eax					# restore EAX
	add $0x8, %esp				# delete initial (now moved) CS and EIP
	andl $0xfffffffd, (irq_prob)		# reset MANUAL_POP bit


irq_change_kstack:
	btl $0x2, (irq_prob)		# check bit 3 (CHANGE_KSTACK)
	jnc irq_resume_kstack
	push %eax					# save EAX
	push %ebx					# save EBX

	mov (current_running_task), %eax
	mov 0x1c(%eax), %eax		# get new kernel stack
	sub $0xc, %eax				# make place for three values

	mov 0x8(%esp), %ebx			# get EIP from current ESP
	mov %ebx, (%eax)			# and store it on the new stack

	mov 0xc(%esp), %ebx			# get CS from current ESP
	mov %ebx, 0x4(%eax)			# and store it on the new stack

	mov 0x10(%esp), %ebx		# get EFLAGS from current ESP
	mov %ebx, 0x8(%eax)			# and store it on the new stack


	mov %esp, %ebx				# store old stack
	mov %eax, %esp				# switch to new stack
	mov %esp, %ebp
	mov %ebx, %eax
	mov (%eax), %ebx			# restore EBX
	mov 0x4(%eax), %eax			# restore EAX

	andl $0xfffffffb, (irq_prob)		# reset CHANGE_KSTACK bit

irq_resume_kstack:
	btl $0x3, (irq_prob)		# check bit 4 (RESUME_KSTACK)
	jnc irq_common_stub_exit
	push %eax					# save EAX
	push %ebx					# save EBX

	mov (current_running_task), %eax
	mov 0x18(%eax), %eax		# get current_running_task's context
	mov 0x2c(%eax), %eax		# get EBP from the context
	mov %eax, %ebp				# update EBP

	mov (current_running_task), %eax
	mov 0x18(%eax), %eax		# get current_running_task's context
	mov 0x28(%eax), %eax		# get ESP from the context
	sub $0xc, %eax				# make place for three values

	mov 0x8(%esp), %ebx			# get EIP from current ESP
	mov %ebx, (%eax)			# and store it on the new stack

	mov 0xc(%esp), %ebx			# get CS from current ESP
	mov %ebx, 0x4(%eax)			# and store it on the new stack

	mov 0x10(%esp), %ebx		# get EFLAGS from current ESP
	mov %ebx, 0x8(%eax)			# and store it on the new stack

	mov %esp, %ebx				# store old stack
	mov %eax, %esp				# switch to new stack
	mov %ebx, %eax
	mov (%eax), %ebx			# restore EBX
	mov 0x4(%eax), %eax			# restore EAX
	andl $0xfffffff7, (irq_prob)		# reset RESUME_KSTACK bit
	jmp irq_common_stub_exit

irq_common_stub_pre_exit:
	add $0x8, %esp				# remove error code and interrupt number
irq_common_stub_exit:
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
