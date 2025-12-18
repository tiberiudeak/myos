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

// the system-wide table of open files
struct open_files_table *open_files_table;

extern char _kernel_end[];

void panic(char *msg) {
	asm volatile("cli");

	printk("\n\n*** KERNEL PANIC ***\n");
	printk("%s", msg);

	for (;;) {
		asm volatile("hlt");
	}
}

void kmain(unsigned long magic, unsigned long addr) {
	int ret;
	uint32_t tmp_vaddr;
	struct multiboot_info *mbi;

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		return;
	}

	// virtual memory setup - phase 2 (after the early boot phase)
	if (vmm_init_phase2()) {
		return;
	}

	// map mbi structure temporarily to access its members
	uint32_t mbi_vaddr = vmm_map_page_early(addr);

	if (!mbi_vaddr) {
		return;
	}

	mbi = (struct multiboot_info *) mbi_vaddr;

	if (mbi->flags & MULTIBOOT_INFO_MEMMAP) {
		uint32_t mmap_vaddr = vmm_map_page_early(mbi->mmap_addr);

		if (!mmap_vaddr) {
			return;
		}

		// initialize physical memory manager
		if (pmm_init(mmap_vaddr, mbi->mmap_length)) {
			return;
		}

		// ignore return code in this case, as this would be a warning
		// but we can't print it at this point
		vmm_unmap_page(mmap_vaddr);
	} else {
		return;
	}

	// init video
	if (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER) {
		ret = tty_init_vbe(mbi);
	} else {
		ret = tty_init_vga();
	}

	if (ret) {
		return;
	}

	tty.terminal_initialize();

	if (mbi->flags & MULTIBOOT_INFO_BOOTLOADNAME) {
		tmp_vaddr = vmm_map_page_early(mbi->bootloader_name);

		if (tmp_vaddr) {
			printk("bootloader booting the kernel: %s\n", (char *) tmp_vaddr);

			if (!vmm_unmap_page(tmp_vaddr)) {
				printk("warning: page could not be unmapped: vaddr = 0x%.8x\n", tmp_vaddr);
			}
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_MEMORY) {
		printk("mem_lower = 0x%xKB, mem_upper = 0x%xKB\n",
				mbi->mem_lower, mbi->mem_upper);
	}

	if (mbi->flags & MULTIBOOT_INFO_BOOTDEV) {
		printk("boot_device = 0x%x\n", mbi->boot_device);
	}

	if (mbi->flags & MULTIBOOT_INFO_CMDLINE) {
		tmp_vaddr = vmm_map_page_early(mbi->cmdline);

		if (tmp_vaddr) {
			printk("cmdline = %s\n", (char *) tmp_vaddr);

			if (!vmm_unmap_page(tmp_vaddr)) {
				printk("warning: page could not be unmapped: vaddr = 0x%.8x\n", tmp_vaddr);
			}
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_MODS && mbi->mods_count != 0) {
		// struct multiboot_mod_list *mod;
		// unsigned int i;

		printk("mod_count = %d, mods_addr = 0x%x\n",
				mbi->mods_count, mbi->mods_addr);

		// for (i = 0, mod = (struct multiboot_mod_list *) mbi->mods_addr;
		// 		i < mbi->mods_count; i++, mod++) {
		// 	printk("mod_start = 0x%x, mod_end = 0x%x, cmdline = %s\n",
		// 			mod->mod_start, mod->mod_end, (char *) mod->string);
		// }
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

		printk("multiboot section header table from the ELF kernel:\n\
sections = %d, size = 0x%x, addr = 0x%x, shndx = 0x%x\n", elf_shdr->num,
				elf_shdr->size, elf_shdr->addr, elf_shdr->shndx);
	}

	if (mbi->flags & MULTIBOOT_INFO_MEMMAP) {
		struct multiboot_mmap_entry *mmap_entry;

		tmp_vaddr = vmm_map_page_early(mbi->mmap_addr);

		if (tmp_vaddr) {
			for (mmap_entry = (struct multiboot_mmap_entry *) tmp_vaddr;
					(unsigned long) mmap_entry < tmp_vaddr + mbi->mmap_length;
					mmap_entry = (struct multiboot_mmap_entry *) ((unsigned long) mmap_entry +
						mmap_entry->size + sizeof(mmap_entry->size))) {
				printk("mem [%8llx-%8llx] %8s\n", mmap_entry->base_addr,
						mmap_entry->base_addr + mmap_entry->length - 1,
						mmap_entry->type == 1 ? "usable" : "reserved");
			}

			if (!vmm_unmap_page(tmp_vaddr)) {
				printk("warning: page could not be unmapped: vaddr = 0x%.8x\n", tmp_vaddr);
			}
		}
	}

	if (mbi->flags & MULTIBOOT_INFO_DRIVE && mbi->drive_length != 0) {
		struct multiboot_drive_entry *drive_entry;

		tmp_vaddr = vmm_map_page_early(mbi->drive_addr);

		if (tmp_vaddr) {
			for (drive_entry = (struct multiboot_drive_entry *) tmp_vaddr;
					(unsigned long) drive_entry < tmp_vaddr + mbi->drive_length;
					drive_entry = (struct multiboot_drive_entry *) ((unsigned long) drive_entry +
						drive_entry->size + sizeof(drive_entry->size))) {
				printk("drive number =  %d, drive_mode = %s\n",
						drive_entry->drive_number, drive_entry->drive_mode == 0 ? "CHS" : "LBA");
			}

			if (!vmm_unmap_page(tmp_vaddr)) {
				printk("warning: page could not be unmapped: vaddr = 0x%.8x\n", tmp_vaddr);
			}
		}
	}

	if (!vmm_unmap_page(mbi_vaddr)) {
		printk("warning: page could not be unmapped: vaddr = 0x%.8x\n", mbi_vaddr);
	}

	// print physical mem info
	print_phymem_info();

	// test page allocator
	test_page_allocator();

	// initialize global descriptor table
	gdt_init();

	// initialize interrupt descriptor table
	idt_init();

	// find and initialize acpi tables
	ret = acpi_init();

	if (ret) {
		panic("ACPI could not be initialized successfully!\n");
	}

	while(1);

	// initialize PS/2 controller
	ret = PS2_init();

	if (ret) {
		panic("");
	}

	// install keyboard irq handler
	keyboard_init();

	// initialize programmable interrupt timer
	PIT_init();

#ifdef CONFIG_TTY_VBE
	ret = map_framebuffer(); // map the framebuffer
	if (ret) {
		panic("");
	}
#endif /* CONFIG_TTY_VBE */

	ret = fs_init(); // initialize the file system

	if (ret) {
		panic("initialize the file system\n");
	}

	open_files_table = init_open_files_table();

	if (open_files_table == NULL) {
		panic("init open files table!\n");
	}

	printk("Welcome to MyOS!\n\n");
	printk("-- type help for available commands --\n\n");
	shell_init(); // initialize the shell

#ifdef CONFIG_FCFS_SCH
	ret = scheduler_init();

	if (ret) {
		panic("initialize the scheduler\n");
	}

	simple_task_scheduler();
#else
	// default Round-Robin
	ret = scheduler_init_rr(); // initialize the round robin scheduler

	if (ret) {
		panic("initialize the scheduler\n");
	}

	// start first process
	start_init_task();
#endif
}
