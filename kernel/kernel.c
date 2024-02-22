#include <kernel/tty.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <stdio.h>
#include <kernel/acpi.h>
#include <arch/i386/ps2.h>
#include <kernel/keyboard.h>
#include <kernel/shell.h>
#include <arch/i386/pit.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

void kmain() {
	terminal_initialize();
	char *a = "kernel";
	printf("Hello, ");
	printfc(3, "%s", a);
	printf(" World!\n\n\n");

	printf("Initializing GDT with tss...");
	init_gdt();
	printf("done\n");

	printf("Initializing IDT...");
	init_idt();
	printf("done\n");

	printf("Detecting ACPI...");
	ACPI_init();

	PS2_init();

	printf("\n");

	keyboard_init();

	PIT_init();

	__asm__ __volatile__ ("movl $0, %eax; int $0x80");
	__asm__ __volatile__ ("movl $1, %eax; int $0x80");
	__asm__ __volatile__ ("movl $2, %eax; int $0x80");

	initialize_memory();
	printf("\n");

	uint8_t ret = initialize_virtual_memory();
	if (ret) {
		printf("Error initializing the virtual memory manager!\n");
	}

	uint32_t *test = (uint32_t*)0x00054123;
	*test = 1;

	printf("Welcome to MyOS!\n");
	shell_init();
}
