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

	terminal_initialize();
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		printk("Invalid magic number: 0x%x\n", magic);
		return;
	}

	mbi = (struct multiboot_info *) addr;
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
		struct multiboot_mmap_entry *mmap_reg;

		for (mmap_reg = (struct multiboot_mmap_entry *) mbi->mmap_addr;
				(unsigned long) mmap_reg < mbi->mmap_addr + mbi->mmap_length;
				mmap_reg = (struct multiboot_mmap_entry *) ((unsigned long) mmap_reg +
					mmap_reg->size + sizeof(mmap_reg->size))) {
			printk("mem [%llx-%llx] %s\n", mmap_reg->base_addr,
					mmap_reg->base_addr + mmap_reg->length - 1,
					mmap_reg->type == 1 ? "usable" : "reserved");
		}
	}

	while(1);
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
