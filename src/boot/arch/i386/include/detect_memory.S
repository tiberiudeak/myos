
# function that uses the INT 0x15, ax=0xE820 call to get the memory map
# inputs: es:di -> pointer to memory map buffer
MMAP_ENT = 0x1000
ACTUAL_MEM = 0x1004
detect_memory:

	mov $0xE820, %eax			# set up INT 0x15, ax=0xE820
	mov $0x0, %ebx				# start at zero
	mov $ACTUAL_MEM, %di		# set di to offset
	mov $24, %ecx				# ask for 24 bytes
	mov $0x534D4150, %edx		# dx="SMAP"
	movl $1, 20(%di)			# set "ignore bit" to 1 (ACPI 3.X)
	xor %bp, %bp				# bp = 0 (counter for number of entries)

	int $0x15					# call INT 0x15, ax=0xE820
	jc detect_memory_error		# if carry flag set, error
	mov $0x534D4150, %edx		# dx="SMAP"
	cmp %edx, %eax				# check if eax is "SMAP"
	jne detect_memory_error		# if not, error
	cmp $20, %ecx				# check if ecx is greater equal than 20
	jl detect_memory_error		# if not, error
	cmp $24, %ecx				# check if ecx is less equal than 24
	jg detect_memory_error		# if not, bug in bios - error

	jmp jmpin

e820lp:
	mov $0xE820, %eax			# eax, ecx get clobbered by int 0x15
	movl $1, 20(%di)			# set "ignore bit" to 1 (ACPI 3.X)
	mov $24, %ecx				# ask for 24 bytes
	int $0x15					# call INT 0x15, ax=0xE820
	jc e820f					# if carry flag set, list complete
	mov $0x534D4150, %edx		# dx="SMAP"

jmpin:
	jcxz skipent				# if cx is zero, skip entry
	cmp $20, %cl				# check for 24B ACPI 3.0 entry
	jbe notext
	testb $1, 20(%di)			# if so, check if "ignore bit" is set
	je skipent

notext:
	mov 8(%di), %ecx			# get lower uint32_t of memory region length
	or 12(%di), %ecx			# or it with upper uint32_t to test for zero
	jz skipent					# if length uint64_t is 0, skip entry
	inc %bp						# good entry, increment bp
	add $24, %di				# increment di by 24 bytes


skipent:
	test %ebx, %ebx				# if ebx = 0, list is complete
	jne e820lp

e820f:
	mov %bp, (MMAP_ENT)			# store number of entries in memory map
	clc							# clear carry flag
	ret

detect_memory_error:
	stc
	push $detm
	call print_string
	add $0x02, %sp
	hlt
	ret

detm:
	.asciz "Det mem error"
