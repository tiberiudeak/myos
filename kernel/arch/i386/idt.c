#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/pic.h>
#include <string.h>
#include <stdio.h>

idt_gate_t idt_entries[256];
idt_ptr_t idt_ptr_reg;

/**
 * @brief Set the idt_entries gate.
 *
 * This function sets the idt_entries gate.
 *
 * @param n        The number of the idt_entries gate.
 * @param handler  The handler of the idt_entries gate.
 */
void set_idt_gate(int n, uint32_t handler, uint16_t selector, uint8_t flags) {
	idt_entries[n].low_offset = low_16(handler);
	idt_entries[n].selector = selector;
	idt_entries[n].reserved = 0;
	idt_entries[n].flags = flags;
	idt_entries[n].high_offset = high_16(handler);
}

/**
 * @brief Initialize the Interrupt Descriptor Table (IDT).
 *
 * This function first initializes the IDT Descriptor with the
 * address of the IDT Entries and the size of the IDT. Then, it
 * initializes the two Programmable Interrupt Controllers (PICs)
 * and sets the IDT gates for the first 32 interrupts and the
 * first 16 IRQs. Finally, it loads the IDT using the lidt
 * instruction and enables interrupts using the sti instruction.
 *
 * The first 32 interrupts are reserved by the CPU and are
 * called exceptions. The first 16 IRQs are reserved by the
 * PICs and are used to handle hardware interrupts.
 */
void init_idt() {
	printf("Initializing IDT");
	idt_ptr_reg.limit = (sizeof(idt_gate_t) * 256) - 1;
	idt_ptr_reg.base = (uint32_t)&idt_entries;

	memset(&idt_entries, 0, sizeof(idt_gate_t) * 256);

	PIC_configure(0x20, 0x28);

	add_isrs_to_idt();
	add_irqs_to_idt();

	__asm__ __volatile__("lidt %0" : : "m" (idt_ptr_reg));
	__asm__ __volatile__("sti");

	printfc(2, "\t\tdone\n");
}
