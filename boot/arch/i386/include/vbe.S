

vbe_get_info:
    push %bp                    # save base pointer
    mov %sp, %bp                # set base poitner to stack pointer

    push %di                    # save DI register




    # get font from VGA BIOS and store it at address 0x5000 using the 
    # BIOS interrupt 0x10 0x1130
    push %bp                    # save BP; the BIOS int 0x10 0x1130 6 will return the address
                                # of ROM 8x16 at ES:BP
    xor %ax, %ax                # AX = 0
    mov %ax, %es                # make ES 0
    mov $0x5000, %di            # store 0x5000 into DI

    push %ds                    # store DS and ES
    push %es

    mov $0x1130, %ax            # move 0x1130 into Ax (get current character generator info)
    mov $6, %bh                 # request code 6: get addr of ROM 8x16 font
    int $0x10                   # perform BIOS interrupt

    push %es                    # store ES to stack
    pop %ds                     # DS will now be ES
    pop %es                     # pop ES

    mov %bp, %si                # move stack pointer to SI
    mov $256, %ax               # store number of ascii characters times 16 bits each
                                # divided by 4 (move 4 bytes at a time)
    mov $16, %cx
    mul %cx
    mov $4, %cx
    div %cx
    mov %ax, %cx

    rep movsd                   # move from ds:esi to es:edi
    pop %ds                     # restore DS
    pop %bp                     # restore BP


    # populate the VbeInfoBlock
    xor %ax, %ax                # set AT to 0
    mov %ax, %es                # set ES to AX
    mov $0x4F, %ah              # AX will be 0x4F00 (get controller info)
    mov $VbeInfoBlock, %di      # store starting address for the info block in DI
    int $0x10                   # call BIOS interrupt

    cmp $0x4F, %ax              # if value in AX != 0x004F, then error
    jne .vbe_error

    mov (VbeInfoBlock+14), %ax  # store video mode pointer offset into AX
    mov %ax, (vbe_offset)
    mov (VbeInfoBlock+16), %ax  # store video mode pointer segment into AX
    mov %ax, (vbe_segment)

    mov %ax, %fs                # store segment into FS
    mov (vbe_offset), %si       # store offset into SI


.vbe_gothrough_modes:
    # go through the modes
    mov %fs:vbe_offset, %bx     # store in BX segment:offset address
    inc %si                     # increment SI to go to the next mode, each mode has
    inc %si                     # two bytes, ending with 0xFFFF (end of list)

    mov %si, (vbe_offset)       # move the new SI into the vbe_offset
    mov (%bx), %ax              # move value in BX (the mode) into AX
    mov %ax, (vbe_mode)         # move AX into vbe_mode

    cmp $0xFFFF, %ax            # check if end of list is reached
    je .vbe_end_modes           # if so, jump to end of modes label

    # get mode info
    mov $0x4F01, %ax            # AX will be 0x4F01 (get mode info)
    mov (vbe_mode), %cx         # store mode in CX
    mov $VbeModeInfoBlock, %di  # store mode info block address into DI
    int $0x10                   # perform BIOS interrupt

    cmp $0x4F, %ax              # if AX != 0x004F, error
    jne .vbe_error


    # print mode information (debug)
    # mov (VbeModeInfoBlock+18), %bx
    # push %bx
    # call print_hex
    # add $0x2, %sp
    # mov $0xE20, %ax
    # int $0x10

    # mov (VbeModeInfoBlock+20), %bx
    # push %bx
    # call print_hex
    # add $0x2, %sp
    # mov $0xE20, %ax
    # int $0x10

    # mov (VbeModeInfoBlock+25), %bx
    # and $0x00FF, %bx
    # push %bx
    # call print_hex
    # add $0x2, %sp
    # mov $0xE0A, %ax
    # int $0x10
    # mov $0x0D, %al
    # int $0x10

    # check if current mode is the one we wanted
    mov (VbeModeInfoBlock+18), %ax  # move the width in AX
    cmp (vbe_width), %ax            # compare the widths
    jne .vbe_goto_next_mode


    mov (VbeModeInfoBlock+20), %ax  # move the height in AX
    cmp (vbe_height), %ax           # compare the heights
    jne .vbe_goto_next_mode

    mov (vbe_bpp), %bx              # store bpp in BX
    and $0x00FF, %bx                # mask the upper byte
    mov (VbeModeInfoBlock+25), %ax  # move the bpp in AX
    and $0x00FF, %ax                # get only the bpp byte
    cmp %bx, %ax                    # compare the bpps
    jne .vbe_goto_next_mode


    # desired mode found, now set it
    mov $0x4F02, %ax                # AX will be 0x4F02 (set video mode)
    mov (vbe_mode), %bx             # move vbe mode into BX
    or $0x4000, %bx                 # enable linear frame buffer
    xor %di, %di                    # set DI to 0
    int $0x10                       # perform BIOS interrupt

    cmp $0x4F, %ax                  # if AX != 0x004F, error
    jne .vbe_error

    # write VbeModeInfoBlock to address 0x4E00 (right before the font)
    mov $VbeModeInfoBlock, %si      # store starting address of VbeModeInfoBlock in SI
    mov $0x4E00, %di                # store info at addr 0x4E00
    mov $128, %cx                   # 512 bytes are 128 double words
    rep movsd                       # copy data

    #mov (0x4E2A), %bx
    #push %bx
    #call print_hex
    #add $0x2, %sp

    pop %di                         # restore registers
    mov %bp, %sp
    pop %bp

    xor %ax, %ax                    # set return code to 0
    ret

.vbe_error:
    pop %di                         # restore registers
    mov %bp, %sp
    pop %bp

    push $.vbe_error_msg
    call print_string
    add $0x2, %sp

    mov $0x1, %ax                   # set return code to 1
    ret

.vbe_goto_next_mode:
    mov (vbe_segment), %ax
    mov %ax, %fs
    mov (vbe_offset), %si
    jmp .vbe_gothrough_modes
    
.vbe_end_modes:
    push $.vbe_end_msg
    call print_string
    add $0x2, %sp
    jmp .vbe_error

.vbe_error_msg:
    .asciz  "vbe error"
.vbe_succ_msg:
    .asciz  "vbe info success"
.vbe_end_msg:
    .asciz  "end of list"

# My VBE information
vbe_width:      .word   1920
vbe_height:     .word   1080
vbe_bpp:        .byte   32
vbe_offset:     .word   0
vbe_segment:    .word   0
vbe_mode:       .word   0

VbeInfoBlock:
    .byte   'V','E','S','A'     # signature: "VESA"
    .word   0x0300              # VbeVersion: 0x0300
    .long   0x0                 # OemStringPtr
    .long   0x0                 # Capabilities
    .long   0x0                 # VideoModePtr
    .word   0x0                 # Total Memory
    .space  222                 # Reserved
    .space  256                 # OemData

VbeModeInfoBlock:
    .word   0x0                 # attributes
    .byte   0x0                 # window_a
    .byte   0x0                 # window_b
    .word   0x0                 # granularity
    .word   0x0                 # window size
    .word   0x0                 # segment_a
    .word   0x0                 # segment_b
    .long   0x0                 #
    .word   0x0                 # bytes per scanline
    .word   0x0                 # width in pixels
    .word   0x0                 # height in pixels
    .byte   0x0                 #
    .byte   0x0                 #
    .byte   0x0                 # planes
    .byte   0x0                 # bits per pixel in this mode
    .byte   0x0                 # banks
    .byte   0x0                 # memory model
    .byte   0x0                 # bank size
    .byte   0x0                 # image pages
    .byte   0x0                 # reserved
    .byte   0x0                 # red mask
    .byte   0x0                 # red position
    .byte   0x0                 # green mask
    .byte   0x0                 # green position
    .byte   0x0                 # blue mask
    .byte   0x0                 # blue position
    .byte   0x0                 # reserved mask
    .byte   0x0                 # reserved position
    .byte   0x0                 # direct color attributes
    .long   0x0                 # framebuffer
    .long   0x0                 # off screen mem off
    .word   0x0                 # off screen mem size
    .space  206                 # reserved


