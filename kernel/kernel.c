#include <kernel/tty.h>
#include <kernel/acpi.h>
#include <kernel/keyboard.h>
#include <kernel/shell.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/ps2.h>
#include <arch/i386/pit.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <fs.h>
#include <string.h>

extern char kernel_end[];

// the system-wide table of open files
open_files_table_t *open_files_table;
inode_block_t *open_inodes_table;

// current available index in the tables defined above
uint8_t current_open_fd = 3;
uint8_t current_open_inode_idx = 3;

void halt_processor(void) {
	__asm__ __volatile__ ("cli; hlt");
}

void kmain() {
	terminal_initialize();      // clear screen
	char *a = "kernel";
	printf("Hello, ");
	printfc(3, "%s", a);
	printf(" World!\n\n");

	uint8_t ret;

	init_gdt();			    // initialize global descriptor table
	init_idt();			    // initialize interrupt descriptor table
	ACPI_init();		    // detect some ACPI tables
	ret = PS2_init();	    // initialize PS/2 controller

	if (ret) {
		printfc(4, "failed");
		halt_processor();
	}

	keyboard_init();	    // install keyboard irq handler
	PIT_init();			    // initialize programmable interrupt timer

	// // test syscalls
	__asm__ __volatile__ ("movl $0, %eax; int $0x80");
	__asm__ __volatile__ ("movl $1, %eax; int $0x80");

	initialize_memory();	// initialize physical memory manager
	printf("\n");

	ret = initialize_virtual_memory();	// initialize virtual memory

	if (ret) {
        halt_processor();
	}

#ifdef CONFIG_TTY_VBE
    ret = map_framebuffer();    // map the framebuffer
    if (ret) {
        halt_processor();
    }
#endif /* CONFIG_TTY_VBE */

    ret = fs_init();            // initialize the file system

    if (ret) {
        printf("Error initializing the file system\n");
        halt_processor();
    }

    open_files_table = init_open_files_table();
    
    if (open_files_table == NULL) {
        printfc(4, "failed to init open files table!\n");
        halt_processor();
    }

    open_inodes_table = init_open_inodes_table();

    if (open_inodes_table == NULL) {
        printfc(4, "failed to init open inodes table!\n");
        halt_processor();
    }

	// // TODO: possible test for paging: see if uint8_t* value at KERNEL_ADDRESS
	// // is the same as 0xC0000000
    int fd = open("test.txt", 1);

    if (fd == -1) {
        printf("file not found kenrel!\n");
    }
    else {
        printf("kernel fd received: %d\n", fd);
    }

    open_files_table_t test = open_files_table[fd];
    printf("test: %x\n", test.address);

    char pp[20];
    char *p = (char*)test.address;
    strncpy(pp, p, 10);
    printf("%s\n", pp);


    fd = open("pr1.o", 1);

    if (fd == -1) {
        printf("file not found kenrel!\n");
    }
    else {
        printf("kernel fd received: %d\n", fd);
    }

    test = open_files_table[fd];
    printf("test: %x\n", test.address);

    printf("%d\n", test.inode->id);
	printf("Welcome to MyOS!\n");
	shell_init();
}

