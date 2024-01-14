
# function that uses the INT 0x15, ax=0xE820 call to get the memory map
#
# input: address where the memory map should be stored as an argument
# on the stack (the actual map will be stored at an offset of 4 bytes from
# this address, because the first 4 bytes are used to store the number of
# entries)

# Layout of one entry in the memory map:
#  64                               32 31                                0
# |-----------------------------------|-----------------------------------|
# |       Region Type (4 bytes)       |           ACPI 3.0 (4 bytes)      |
# |-----------------------------------|-----------------------------------|
# |                Length of Memory Region (8 bytes)                      |
# |-----------------------------------------------------------------------|
# |              Base Address of Memory Region (8 bytes)                  |
# |-----------------------------------------------------------------------|

# Region Type:
# 1 = Usable RAM
# 2 = Reserved - unusable
# 3 = ACPI reclaimable memory
# 4 = ACPI NVS memory
# 5 = Area containing bad memory

# ACPI 3.0:
# bit 0: 1 = ignore this entry when calculating memory
# bit 1: 1 = if the entry in non-volatile
# rest of bits: undeifned


detect_memory:

	push %bp                    # save base pointer
	mov %sp, %bp                # set base pointer to stack pointer

	# save registers that will be modified
	push %ax
	push %bx
	push %cx
	push %dx
	push %di

	mov $0xE820, %eax           # set up INT 0x15, ax=0xE820
	mov $0x0, %ebx              # start at zero
	mov 4(%bp), %di             # set di to offset
	add $4, %di                 # add 4 to di to skip over the # of entries
	mov $24, %ecx               # ask for 24 bytes
	mov $0x534D4150, %edx       # dx="SMAP"
	movl $1, 20(%di)            # set "ignore bit" to 1 (ACPI 3.X)
	xor %si, %si                # si = 0 (counter for number of entries)

	int $0x15                   # call INT 0x15, ax=0xE820
	jc detect_memory_error      # if carry flag set, error
	mov $0x534D4150, %edx       # dx="SMAP"
	cmp %edx, %eax              # check if eax is "SMAP"
	jne detect_memory_error     # if not, error
	cmp $20, %ecx               # check if ecx is greater equal than 20
	jl detect_memory_error      # if not, error
	cmp $24, %ecx               # check if ecx is less equal than 24
	jg detect_memory_error      # if not, bug in bios - error

	jmp jmpin

e820lp:
	mov $0xE820, %eax           # eax, ecx get clobbered by int 0x15
	movl $1, 20(%di)            # set "ignore bit" to 1 (ACPI 3.X)
	mov $24, %ecx               # ask for 24 bytes
	int $0x15                   # call INT 0x15, ax=0xE820
	jc e820f                    # if carry flag set, list complete
	mov $0x534D4150, %edx       # dx="SMAP"

jmpin:
	jcxz skipent                # if cx is zero, skip entry
	cmp $20, %cl                # check for 24B ACPI 3.0 entry
	jbe notext
	testb $1, 20(%di)           # if so, check if "ignore bit" is set
	je skipent

notext:
	mov 8(%di), %ecx            # get lower uint32_t of memory region length
	or 12(%di), %ecx            # or it with upper uint32_t to test for zero
	jz skipent                  # if length uint64_t is 0, skip entry
	inc %si                     # good entry, increment si
	add $24, %di                # increment di by 24 bytes


skipent:
	test %ebx, %ebx             # if ebx = 0, list is complete
	jne e820lp

e820f:
	mov 4(%bp), %bx             # get address where the number of entries is stored
	mov %si, (%bx)              # store number of entries in memory map
	clc                         # clear carry flag
	jmp detect_memory_done      # return

detect_memory_error:
	stc
	push $detm
	call print_string
	add $0x02, %sp
	hlt

detect_memory_done:
	# restore registers
	pop %di
	pop %dx
	pop %cx
	pop %bx
	pop %ax
	mov %bp, %sp
	pop %bp
	ret

detm:
	.asciz "Det mem error"
