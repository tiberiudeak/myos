#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H 1

#include <stdint.h>

#define GDT_ENTRIES 6

/**
 * GDT Descriptor structure. For more details, see the comments
 * in the boot/arch/i386/include/gdt.S file.
 */
typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/**
 * GDT Entry structure. For more details, see the comments
 * in the boot/arch/i386/include/gdt.S file.
*/
typedef struct {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) gdt_entry_t;

/**
 * Task State Segment (TSS) Entry structure that holds information
 * about a task, including general purpose registers, segment selectors,
 * the instruction pointer, the EFLAGS register and control register 3.
*/
typedef struct {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed)) tss_entry_t;

void init_gdt();
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_flush(uint32_t gdt_ptr);
void write_tss(int num, uint16_t ss0, uint32_t esp0);

#endif // KERNEL_GDT_H
