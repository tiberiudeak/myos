#ifndef ARCH_I386_GDT_H
#define ARCH_I386_GDT_H 1

#include <stdint.h>

#define GDT_ENTRIES 6
#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

/**
 * GDT Descriptor structure. For more details, see the comments
 * in the boot/arch/i386/include/gdt.S file.
 */
struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

/**
 * GDT Entry structure. For more details, see the comments
 * in the boot/arch/i386/include/gdt.S file.
 */
struct gdt_entry {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed));

/**
 * Task State Segment (TSS) Entry structure that holds information
 * about a task, including general purpose registers, segment selectors,
 * the instruction pointer, the EFLAGS register and control register 3.
 */
struct tss_entry {
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
} __attribute__((packed));

/**
 * GDT Entry access flags. Layout:
 *
 *   7   6 5   4   3   2    1   0
 * |-------------------------------|
 * | P | DPL | S | E | DC | RW | A |
 * |-------------------------------|
 *
 * P: Present bit, must be 1 for all valid selectors
 * DPL: Descriptor Privilege Level, 0 for kernel, 3 for user
 * S: Descriptor type, 1 for code or data, 0 for system (TSS, LDT)
 * E: Executable bit, 1 for code selectors, 0 for data selectors
 * DC: Direction bit / Conforming bit
 *    - for data selectors, 0 segment grows up, 1 segment grows down
 *    - for code selectors, 0 code in this segment ca only be executed from the ring
 *      set in DPL, 1 code in this segment can be executed from ring DPL and below
 * RW: Readable bit / Writable bit
 *   - for data selectors, 1 segment is writeable, 0 write is not allowed
 *   - for code selectors, 1 segment is readable, 0 read is not allowed
 * A: Accessed bit, set by the CPU when the segment is accessed
 */
typedef enum {
	GDT_ACCESS_CODE_READABLE 		= 0x02,
	GDT_ACCESS_CODE_WRITEABLE 		= 0x02,
	GDT_ACCESS_CODE_CONFORMING 		= 0x04,

	GDT_ACCESS_DATA_SEGMENT 		= 0x10,
	GDT_ACCESS_CODE_SEGMENT 		= 0x18,
	GDT_ACCESS_TSS 					= 0x09,

	GDT_ACCESS_RING0 				= 0x00,
	GDT_ACCESS_RING1 				= 0x20,
	GDT_ACCESS_RING2 				= 0x40,
	GDT_ACCESS_RING3 				= 0x60,

	GDT_ACCESS_PRESENT 				= 0x80,
} GDT_ACCESS;

/**
 * GDT Entry flags. Layout:
 *
 *   3    2    1      0
 * |------------------------|
 * | G | D/B | L | Reserved |
 * |------------------------|
 *
 * G: Granularity bit, 0 for 1 byte, 1 for 4KB
 * D/B: Size flag, 0 for 16-bit, 1 for 32-bit
 * L: Long mode, 1 for 64-bit mode, 0 for other
 */
typedef enum {
	GDT_FLAGS_16_BIT 				= 0x00,
	GDT_FLAGS_32_BIT 				= 0x40,
	GDT_FLAGS_64_BIT 				= 0x20,
	GDT_FLAGS_GRANULARITY_BYTE 		= 0x00,
	GDT_FLAGS_GRANULARITY_4KB 		= 0x80
} GDT_FLAGS;

void init_gdt();
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_flush(uint32_t gdt_ptr);
void write_tss(int num, uint16_t ss0, uint32_t esp0);

#endif // ARCH_I386_GDT_H
