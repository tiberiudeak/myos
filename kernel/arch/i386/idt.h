#ifndef ARCH_I386_INT_H
#define ARCH_I386_INT_H 1

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
	uint8_t always0;
	uint8_t flags;
	uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

#endif // ARCH_I386_INT_H
