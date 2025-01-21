#include <arch/i386/gdt.h>
#include <kernel/string.h>
#include <kernel/tty.h>

extern void gdt_flush(uint32_t);
extern void tss_flush();

struct gdt_entry gdt_entries[GDT_ENTRIES];
struct gdt_ptr gdt_ptr;
struct tss_entry tss_entry;

/**
 * @brief Initialize the Global Descriptor Table (GDT).
 *
 * This function initializes the GDT with the following entries:
 * 0: Null segment
 * 1: Kernel code segment
 * 2: Kernel data segment
 * 3: User code segment
 * 4: User data segment
 * 5: Task State Segment (TSS)
 *
 * The GDT is then loaded into the processor using the gdt_flush function
 * and the TSS is loaded using the tss_flush function.
 */
void init_gdt() {
#ifdef CONFIG_VERBOSE
	printk("Initializing GDT");
#endif
	gdt_ptr.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
	gdt_ptr.base = (uint32_t) &gdt_entries;

	// Null segment
	gdt_set_gate(0, 0, 0, 0, 0);

	// Kernel code segment
	gdt_set_gate(1, 0, 0xFFFFFFFF,
				 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
					 GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
				 GDT_FLAGS_GRANULARITY_4KB | GDT_FLAGS_32_BIT);
	// Kernel data segment
	gdt_set_gate(2, 0, 0xFFFFFFFF,
				 GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 |
					 GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_CODE_WRITEABLE,
				 GDT_FLAGS_GRANULARITY_4KB | GDT_FLAGS_32_BIT);

	// User code segment
	gdt_set_gate(3, 0, 0xFFFFFFFF,
				 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
					 GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE,
				 GDT_FLAGS_GRANULARITY_4KB | GDT_FLAGS_32_BIT);

	// User data segment
	gdt_set_gate(4, 0, 0xFFFFFFFF,
				 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 |
					 GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_CODE_WRITEABLE,
				 GDT_FLAGS_GRANULARITY_4KB | GDT_FLAGS_32_BIT);

	// Task State Segment (TSS)
	write_tss(5, 0x10, 0x90000);

	gdt_flush((uint32_t) &gdt_ptr);
	tss_flush();

#ifdef CONFIG_VERBOSE
	printkc(2, "\t\tdone\n");
#endif
}

/**
 * @brief Set the GDT gate.
 *
 * This function sets the GDT gate with the given parameters.
 *
 * @param num    The number of the GDT gate.
 * @param base   The 32-bit base address of the GDT gate.
 * @param limit  The 32-bit limit of the GDT gate.
 * @param access The access byte of the GDT gate.
 * @param flags  The flags byte of the GDT gate.
 */
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access,
				  uint8_t flags) {
	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;

	gdt_entries[num].limit = (limit & 0xFFFF);
	gdt_entries[num].flags = (limit >> 16) & 0x0F;

	gdt_entries[num].flags |= flags & 0xF0;
	gdt_entries[num].access = access;
}

/**
 * @brief Write the Task State Segment (TSS) entry.
 *
 * This function creates a new TSS entry in the GDT with the given
 * parameters and initializes the TSS entry with the given stack
 * segment and stack pointer.
 *
 * @param num  The number of the TSS entry in the GDT
 * @param ss0  The stack segment for privilege level 0
 * @param esp0 The stack pointer for privilege level 0
 */
void write_tss(int num, uint16_t ss0, uint32_t esp0) {
	uint32_t base = (uint32_t) &tss_entry;
	uint32_t limit = base + sizeof(tss_entry);

	gdt_set_gate(num, base, limit,
				 GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_TSS, 0X00);

	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.ss0 = ss0;   // set the kernel stack segment
	tss_entry.esp0 = esp0; // set the kernel stack pointer

	tss_entry.cs = 0x08 | 0x03; // set the code segment ored with 3 (for ring 3)
	// set other segments to the data segment ored with 3
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs =
		0x10 | 0x03;
}
