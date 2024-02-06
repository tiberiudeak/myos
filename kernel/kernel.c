#include <kernel/tty.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <string.h>
#include <stdio.h>

void test() {
	char* video_memory = (char*) 0xb8000;
	*video_memory = 'A';
	*(video_memory + 2) = 'Y';
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

	printf("\n");
	printf("test: %d\n", 156);
	printf("test: %x\n", 156);
	// printf("%d", 1/0);

	int *ptr = 0x1000;

	printf("ptr: %d\n", *ptr);



	// int aa = 1/0; // get division by zero exception
	// __asm__("int $0xA");
}
