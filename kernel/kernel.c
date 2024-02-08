#include <kernel/tty.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <string.h>
#include <stdio.h>
#include <kernel/io.h>
#include <arch/i386/irq.h>
#include <kernel/acpi.h>

void test() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
	*(video_memory + 2) = 'Y';
}

void bla() {
	printf("bla\n");
	uint8_t scancode = port_byte_in(0x60);
	printf("scancode: %d\n", scancode);
}

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

	printf("\n");
	printf("test: %d\n", 156);
	printf("test: %x\n", 156);
	// printf("%d", 1/0);

	int *ptr = (int*)0x1000;

	printf("ptr: %d\n", *ptr);

	irq_install_handler(1, bla);


	// int aa = 1/0; // get division by zero exception
	// __asm__("int $0xA");
}
