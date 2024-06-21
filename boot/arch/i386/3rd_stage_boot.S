
.section .text
.global _start3
.code16

_start3:
    push $m
    call print_string
    add $0x2, %sp
    call move_cursor_nl

	# at this point, AX will have the number of sectors for the data bitmap
	# and DI will have the starting address for the data bitmap blocks
	# information that will be used in the third stage of the bootloader
	# to load the inodes and finally the kernel data
	mov %ax, %cx			# save AX into CX

	# load inode blocks into main memory
	pop %si					# pop drive number from stack
	mov (0x8C0C), %ax		# first inode block
	mov $8, %bx				# put 8 into BX to get the sector number
	mul %bx					# AX = AX * 8 (sector number)
	mov %ax, %dx			# put result into DX
	inc %dx					# sector count starts from 1 so add 1
	push %dx				# push starting sector to the stack

	# calculate starting address for the inode block
	mov $512, %ax			# move size of a sector into AX
	mul %cx					# multiply that size with the number of sectors
							# occupied by the inode bitmap
	add %di, %ax			# add total size of the inode bitmap to the
							# starting address of the inode bitmap in order
							# to get the starting address for the data bitmap
	push %ax				# push the resulted address to the stack
	mov %ax, %di			# save starting address in DI

	mov (0x8C14), %ax		# number of blocks for the inodes
	mov $8, %bx				# each block has 8 sectors
	mul %bx					# AX = AX * 8 (number of sectors)
	push %ax				# push number of sectors to stack

	push %si				# push drive number to stack

	call disk_load			# call disk load function
	add $0x8, %sp			# restore stack pointer
	push %si				# push drive number back to stack

    call move_cursor_nl

	# at this point, AX will have the number of sectors loaded for the inode blocks
	# and DI will have the starting address for the inode blocks
	mov %ax, %cx			# save AX into CX


	# load kernel data into main memory
	pop %si					# pop drive number from stack
	mov (0x8C10), %ax		# first data block
	inc %ax					# go to kernel data (skip root dir)
	mov $8, %bx				# put 8 into BX to get the sector number
	mul %bx					# AX = AX * 8 (sector number)
	mov %ax, %dx			# put result into DX
	inc %dx					# sector count starts from 1 so add 1
	push %dx				# push starting sector to the stack

	push $0xF000			# starting address for the kernel data is 0xF000

	add $0x80, %di			# DI, that previously contains the starting address
							# for the inode block, will now point to the starting
							# address for the second inode (kernel) (skip first
							# two inodes)
	add $0x9, %di			# go to size in sectors information about the kernel
	mov (%di), %ax			# store kernel's size in sectors in AX
	push %ax				# push total number of sectors to stack

	push %si				# push drive number to stack

	call disk_load			# call disk load function
	add $0x8, %sp			# restore stack pointer
	push %si				# push drive number back to stack

    call move_cursor_nl

	# empty stack
	add $0x2, %sp

    push $swith_pr_mode_msg
    call print_string
    add $0x2, %sp

	# make switch to protected mode
	cli						# disable interrupts
	lgdt gdt_descriptor		# load the gdt descriptor

	mov %cr0, %eax			# move the value of cr0 to eax
	or $0x1, %eax			# set the first bit of eax to 1 (protected mode)
	mov %eax, %cr0			# move the value of eax to cr0

    #hlt
    #aaa: jmp aaa

	ljmp $CODE_SEG, $protected_mode_init	# jump to protected mode


.include "./include/gdt.S"
.include "./include/print_string.S"
#.include "./include/print_hex.S"
.include "./include/disk_load.S"
.include "./include/cursor_next_line.S"

m:
	.asciz "[INFO] Entered 3rd stage boot"

swith_pr_mode_msg:
    .asciz "[INFO] Switching to Protected Mode..."

.code32
protected_mode_init:
	mov $DATA_SEG, %ax	        # load data segment into ax
	mov %ax, %ds                # load ax into data segment register
	mov %ax, %es                # load ax into extra segment register
	mov %ax, %fs                # load ax into fs segment register
	mov %ax, %gs                # load ax into gs segment register
	mov %ax, %ss                # load ax into stack segment register

	mov $0x90000, %ebp			# set the base pointer to 0x90000
	mov %ebp, %esp				# set the stack pointer to base pointer

    #mov (0x4E28), %ebx
    #mov $0xFFFFFFFF, (%ebx)

	call 0xF000					# jump to kernel code

g:
	hlt
	jmp g

.space 512 - (. - _start3)		# fill the rest of the bytes with 0
