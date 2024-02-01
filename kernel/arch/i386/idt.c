#include "idt.h"

idt_gate_t idt[256];

/**
 * @brief Set the IDT gate.
 *
 * This function sets the IDT gate.
 *
 * @param n        The number of the IDT gate.
 * @param handler  The handler of the IDT gate.
 */
void set_idt_gate(int n, uint32_t handler) {
	idt[n].low_offset = low_16(handler);
	idt[n].selector = KERNEL_CS;
	idt[n].always0 = 0;
	idt[n].flags = 0x8E;
	idt[n].high_offset = high_16(handler);
}
