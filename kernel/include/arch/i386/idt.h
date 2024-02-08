#ifndef ARCH_I386_IDT_H
#define ARCH_I386_IDT_H 1

#include <stdint.h>

/**
 * A Segment Selector is a 16-bit value used in Protected and Long Modes to
 * index descriptors in the GDT or LDT. Layout:
 *
 *  15                                 3 2  1   0
 * |---------------------------------------------|
 * |               index                |TI| RPL |
 * |---------------------------------------------|
 *
 * index: 13-bit index representing the index of the GDT or LDT entry referenced
 * 	      by this selector
 * TI: table indicator, 0 for GDT, 1 for LDT
 * RPL: requested privilege level, 2-bit value representing the privilege level
 * 	    required to execute the segment referenced by this selector
 */
#define KERNEL_CS 0x0008
#define low_16(address) (uint16_t)((address) & 0xFFFF)
#define high_16(address) (uint16_t)(((address) >> 16) & 0xFFFF)

/**
 * The IDT consists of 256 descriptor entries, each corresponding to
 * a specific interrupt vector. Each entry is 8 bytes long and has
 * the following structure:
 *
 * 31                                 16 15 14 13 12 11 10 8 7               0
 * |------------------------------------|-------------------------------------|
 * |        offset (16-31)              |P | DPL | 0| D|Type|   always0 (0-7) |
 * |------------------------------------|-------------------------------------|
 * |        selector (0-15)             |           offset (0-15)             |
 * |------------------------------------|-------------------------------------|
 *
 * offset: the 32-bit offset representing the memory address of the interrupt
 *         handler function within the code segment
 * selector: the 16-bit code segment selector in the GDT (see above its layout)
 * P: present bit, must be 1 for the descriptor to be valid
 * DPL: 2-bit value representing the descriptor privilege level
 * D: indicates whether the code segment is 32 bit, set to 1 for 32-bit code
 * Type: 3-bit value representing the gate type, 110 for interrupt gates
 */
typedef struct {
	uint16_t low_offset;
	uint16_t selector;
	uint8_t reserved;
	uint8_t flags;
	uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idt_ptr_t;

/**
 * IDT Entry flags. Layout:
 *
 *   7   6 5   4   3       0
 * |-------------------------|
 * | P | DPL | 0 | Gate Type |
 * |-------------------------|
 *
 * P: Present bit, must be 1 for all valid selectors
 * DPL: Descriptor Privilege Level
 * Gate Type: 4-bit value representing the type of the gate; there
 * 		  are 5 types of gates: task, 16-bit interrupt, 16-bit
 * 		  trap, 32-bit interrupt, 32-bit trap
*/
typedef enum {
	IDT_FLAGS_TASK_GATE 		= 0x05,
	IDT_FLAGS_16BIT_INT 		= 0x06,
	IDT_FLAGS_16BIT_TRAP 		= 0x07,
	IDT_FLAGS_32BIT_INT 		= 0x0E,
	IDT_FLAGS_32BIT_TRAP 		= 0x0F,

	IDT_FLAGS_RING0 			= 0x00,
	IDT_FLAGS_RING1 			= 0x20,
	IDT_FLAGS_RING2 			= 0x40,
	IDT_FLAGS_RING3 			= 0x60,

	IDT_FLAGS_PRESENT 			= 0x80
} IDT_FLAGS;

void set_idt_gate(int n, uint32_t handler, uint16_t selector, uint8_t flags);
void init_idt();

#endif // ARCH_I386_IDT_H
