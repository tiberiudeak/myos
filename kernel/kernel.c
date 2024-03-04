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
#include <stdio.h>

extern char kernel_end[];

void halt_processor(void) {
	__asm__ __volatile__ ("cli; hlt");
}

void kmain() {
	terminal_initialize();	// clear screen
	char *a = "kernel";
	printf("Hello, ");
	printfc(3, "%s", a);
	printf(" World!\n\n");

	uint8_t ret;

	init_gdt();			// initialize global descriptor table
	init_idt();			// initialize interrupt descriptor table
	ACPI_init();		// detect some ACPI tables
	ret = PS2_init();	// initialize PS/2 controller

	if (ret) {
		printfc(4, "failed");
		halt_processor();
	}

	keyboard_init();	// install keyboard irq handler
	PIT_init();			// initialize programmable interrupt timer

	// test syscalls
	__asm__ __volatile__ ("movl $0, %eax; int $0x80");
	__asm__ __volatile__ ("movl $1, %eax; int $0x80");

	initialize_memory();	// initialize physical memory manager
	printf("\n");

	ret = initialize_virtual_memory();	// initialize virtual memory

	if (ret) {
		printf("Error initializing the virtual memory manager!\n");
		halt_processor();
	}

	// TODO: possible test for paging: see if uint8_t* value at KERNEL_ADDRESS
	// is the same as 0xC0000000

	printf("Welcome to MyOS!\n");
	shell_init();
}
