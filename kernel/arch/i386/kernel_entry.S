
# kernel entry point that is executed after the bootloader
# calls the kernel main function
.code32
.global _start
.extern kmain
.section kernel_entry

_start:
	call kmain			# call kernel main

hang:					# hang forever
	jmp hang
