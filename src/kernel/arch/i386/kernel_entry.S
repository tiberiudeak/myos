.code32
.global main

call main				# call kernel main

hang:					# hang forever
	jmp hang
