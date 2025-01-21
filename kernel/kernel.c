#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/pit.h>
#include <arch/i386/ps2.h>
#include <kernel/acpi.h>
#include <kernel/elf.h>
#include <kernel/fs.h>
#include <kernel/keyboard.h>
#include <kernel/shell.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <process/scheduler.h>

#include <stdint.h>

extern char kernel_end[];

// the system-wide table of open files
struct open_files_table *open_files_table;

void halt_processor(void) {
	while (1) {
		__asm__ __volatile__("cli; hlt");
	}
}

void kmain() {
	terminal_initialize(); // clear screen
#ifdef CONFIG_VERBOSE
	char *a = "kernel";
	printk("Booting, ");
	printkc(3, "%s", a);
	printk("...\n\n");
#endif

	int8_t ret;

	init_gdt();		  // initialize global descriptor table
	init_idt();		  // initialize interrupt descriptor table
	ACPI_init();	  // detect some ACPI tables
	ret = PS2_init(); // initialize PS/2 controller

	if (ret) {
#ifdef CONFIG_VERBOSE
		printkc(4, "failed");
#endif
		halt_processor();
	}

	keyboard_init(); // install keyboard irq handler
	PIT_init();		 // initialize programmable interrupt timer

	ret = initialize_memory(); // initialize physical memory manager

	if (ret) {
		printk("Error initializing the physical memory manager\n");
		halt_processor();
	}

	ret = initialize_virtual_memory(); // initialize virtual memory

	if (ret) {
		printk("Error initializing the virtual memory manager\n");
		halt_processor();
	}

#ifdef CONFIG_TTY_VBE
	ret = map_framebuffer(); // map the framebuffer
	if (ret) {
		halt_processor();
	}
#endif /* CONFIG_TTY_VBE */

	ret = fs_init(); // initialize the file system

	if (ret) {
		printk("Error initializing the file system\n");
		halt_processor();
	}

	open_files_table = init_open_files_table();

	if (open_files_table == NULL) {
		printkc(4, "failed to init open files table!\n");
		halt_processor();
	}

	printk("Welcome to MyOS!\n\n");
	printk("-- type help for available commands --\n\n");
	shell_init(); // initialize the shell

#ifdef CONFIG_FCFS_SCH
	ret = scheduler_init();

	if (ret) {
		printkc(4, "failed to initialize the scheduler\n");
		halt_processor();
	}

	simple_task_scheduler();
#else
	// default Round-Robin
	ret = scheduler_init_rr(); // initialize the round robin scheduler

	if (ret) {
		printkc(4, "failed to initialize the scheduler\n");
		halt_processor();
	}

	// start first process
	start_init_task();
#endif
}
