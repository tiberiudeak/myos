#include <kernel/tty.h>
#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <stdio.h>
#include <kernel/acpi.h>
#include <arch/i386/ps2.h>
#include <kernel/keyboard.h>
#include <kernel/shell.h>

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

	printf("Welcome to MyOS!\n");
	shell_init();
}
