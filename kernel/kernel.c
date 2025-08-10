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
#include <kernel/multiboot.h>
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

void kmain(unsigned long magic, unsigned long addr) {
	struct multiboot_info *mbi;

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		return;
	}

	mbi = (struct multiboot_info *) addr;

	// init video
	if (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER) {
		tty_init_vbe();
	} else {
		tty_init_vga();
	}

	tty.terminal_initialize();

	printk("flags = 0x%x\n", mbi->flags);

	if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
		printk("mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
				mbi->mem_lower, mbi->mem_upper);
	}

	if (mbi->flags & MULTIBOOT_INFO_BOOTDEV) {
		printk("boot_device = 0x%x\n", mbi->boot_device);
	}

	if (mbi->flags & MULTIBOOT_INFO_CMDLINE) {
		printk("cmdline = %s\n", (char *) mbi->cmdline);
	}

	if (mbi->flags & MULTIBOOT_INFO_MODS && mbi->mods_count != 0) {
		struct multiboot_mod_list *mod;
		unsigned int i;

		printk("mod_count = %d, mods_addr = 0x%x\n",
				mbi->mods_count, mbi->mods_addr);

		for (i = 0, mod = (struct multiboot_mod_list *) mbi->mods_addr;
				i < mbi->mods_count; i++, mod++) {
			printk("mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
					mod->mod_start, mod->mod_end, (char *) mod->string);
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_SYMS && mbi->flags & MULTIBOOT_INFO_ELF_SHDR) {
		printk("both bits 4 and 5 are set in the multiboot flags!\n");
		return;
	}

	if (mbi->flags & MULTIBOOT_INFO_SYMS) {
		struct multiboot_sym_table *sym_table = &(mbi->u.sym_table);

		if (sym_table->tabsize != 0) {
			printk("symbol table from the a.out kernel image: tabsize = 0x%x, strsize = 0x%x, addr = 0x%x\n",
				sym_table->tabsize, sym_table->strsize, sym_table->addr);
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_ELF_SHDR) {
		struct multiboot_elf_shdr *elf_shdr = &(mbi->u.elf_shdr);

		printk("multiboot section header table from the ELF kernel, sections = %d,\
				size = 0x%x, addr = 0x%x, shndx = 0x%x\n", elf_shdr->num,
				elf_shdr->size, elf_shdr->addr, elf_shdr->shndx);
	}

	if (mbi->flags & MULTIBOOT_INFO_MEMMAP) {
		struct multiboot_mmap_entry *mmap_entry;

		for (mmap_entry = (struct multiboot_mmap_entry *) mbi->mmap_addr;
				(unsigned long) mmap_entry < mbi->mmap_addr + mbi->mmap_length;
				mmap_entry = (struct multiboot_mmap_entry *) ((unsigned long) mmap_entry +
					mmap_entry->size + sizeof(mmap_entry->size))) {
			printk("mem [%llx-%llx] %s\n", mmap_entry->base_addr,
					mmap_entry->base_addr + mmap_entry->length - 1,
					mmap_entry->type == 1 ? "usable" : "reserved");
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_DRIVE && mbi->drive_length != 0) {
		struct multiboot_drive_entry *drive_entry;

		for (drive_entry = (struct multiboot_drive_entry *) mbi->drive_addr;
				(unsigned long) drive_entry < mbi->drive_addr + mbi->drive_length;
				drive_entry = (struct multiboot_drive_entry *) ((unsigned long) drive_entry +
					drive_entry->size + sizeof(drive_entry->size))) {
			printk("drive number =  %d, drive_mode = %s\n",
					drive_entry->drive_number, drive_entry->drive_mode == 0 ? "CHS" : "LBA");
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_BOOTLOADNAME) {
		printk("bootloader booting the kernel: %s\n", (char *) mbi->bootloader_name);
	}


	while(1);
#ifdef CONFIG_VERBOSE
	char *a = "kernel";
	printk("Booting, ");
	printk("%s", a);
	printk("...\n\n");
#endif

	int8_t ret;

	init_gdt();		  // initialize global descriptor table
	init_idt();		  // initialize interrupt descriptor table
	ACPI_init();	  // detect some ACPI tables
	ret = PS2_init(); // initialize PS/2 controller

	if (ret) {
#ifdef CONFIG_VERBOSE
		printk("failed");
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
		printk("failed to init open files table!\n");
		halt_processor();
	}

	printk("Welcome to MyOS!\n\n");
	printk("-- type help for available commands --\n\n");
	shell_init(); // initialize the shell

#ifdef CONFIG_FCFS_SCH
	ret = scheduler_init();

	if (ret) {
		printk("failed to initialize the scheduler\n");
		halt_processor();
	}

	simple_task_scheduler();
#else
	// default Round-Robin
	ret = scheduler_init_rr(); // initialize the round robin scheduler

	if (ret) {
		printk("failed to initialize the scheduler\n");
		halt_processor();
	}

	// start first process
	start_init_task();
#endif
}
