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
#include <tests/tests.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <fs.h>
#include <string.h>
#include <elf.h>

extern char kernel_end[];

// the system-wide table of open files
open_files_table_t *open_files_table;

void halt_processor(void) {
	__asm__ __volatile__ ("cli; hlt");
}

void kmain() {
	terminal_initialize();      // clear screen
	char *a = "kernel";
	printk("Hello, ");
	printkc(3, "%s", a);
	printk(" World!\n\n");

	int8_t ret;

	init_gdt();			    // initialize global descriptor table
	init_idt();			    // initialize interrupt descriptor table
	ACPI_init();		    // detect some ACPI tables
	ret = PS2_init();	    // initialize PS/2 controller

	if (ret) {
		printkc(4, "failed");
		halt_processor();
	}

	keyboard_init();	    // install keyboard irq handler
	PIT_init();			    // initialize programmable interrupt timer

	// // test syscalls
	__asm__ __volatile__ ("movl $0, %eax; int $0x80");
	__asm__ __volatile__ ("movl $1, %eax; int $0x80");

	initialize_memory();	// initialize physical memory manager
	printk("\n");

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
        printk("Error initializing the file system\n");
        halt_processor();
    }

    open_files_table = init_open_files_table();
    
    if (open_files_table == NULL) {
        printkc(4, "failed to init open files table!\n");
        halt_processor();
    }

	// // TODO: possible test for paging: see if uint8_t* value at KERNEL_ADDRESS
	// // is the same as 0xC0000000
    ret = test_open_close_syscalls();

    if (ret) {
        printkc(4, "open and close syscalls test failed!\n");
        halt_processor();
    }
 
    int aaa = 1;
    printf("test %d from printf function\nbla? %d", aaa, aaa);
    ret = fflush();

    if (ret == -1) {
        printk("error wirn\n");
    }

    execute_elf("pr1");

	printk("Welcome to MyOS!\n");
	shell_init();
}

