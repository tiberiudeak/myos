#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <kernel/io.h>
#include <kernel/tty.h>

void *irq_routines[16] = { 0 };

/**
 * @brief Install a new handler for the given IRQ.
 *
 * This function installs a new handler for the given IRQ.
 *
 * @param irq      The IRQ number.
 * @param handler  The handler function.
 */
void irq_install_handler(int irq, void (*handler)(interrupt_regs *r)) {
	irq_routines[irq] = handler;
}

/**
 * @brief Uninstall the handler for the given IRQ.
 *
 * This function uninstalls the handler for the given IRQ.
 *
 * @param irq  The IRQ number.
 */
void irq_uninstall_handler(int irq) {
	irq_routines[irq] = 0;
}

/**
 * @brief Interrupt Request (IRQ) handler.
 *
 * This function is called when an interrupt request is received.
 * It calls the handler for the given IRQ.
 *
 * @param r  The interrupt registers.
 */
void irq_handler(interrupt_regs *r) {
	void (*handler)(interrupt_regs *r);
	handler = irq_routines[r->int_no - 32];
	if (handler) {
		handler(r);
	}

	if (r->int_no >= 8) {
		port_byte_out(0xA0, 0x20);
	}
	port_byte_out(0x20, 0x20);
}

/**
 * @brief Add the Interrupt Request (IRQ) handlers to the IDT.
 *
 * This function adds the Interrupt Request (IRQ) handlers to the
 * Interrupt Descriptor Table (IDT).
 */
void add_irqs_to_idt() {
	set_idt_gate(32, (uint32_t)irq0, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(33, (uint32_t)irq1, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(34, (uint32_t)irq2, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(35, (uint32_t)irq3, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(36, (uint32_t)irq4, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(37, (uint32_t)irq5, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(38, (uint32_t)irq6, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(39, (uint32_t)irq7, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(40, (uint32_t)irq8, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(41, (uint32_t)irq9, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(42, (uint32_t)irq10, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(43, (uint32_t)irq11, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(44, (uint32_t)irq12, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(45, (uint32_t)irq13, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(46, (uint32_t)irq14, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(47, (uint32_t)irq15, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
}
