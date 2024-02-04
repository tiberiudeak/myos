#include <kernel/idt.h>
#include <kernel/io_port.h>
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
void set_idt_gate(int n, uint32_t handler) {
	idt_entries[n].low_offset = low_16(handler);
	idt_entries[n].selector = KERNEL_CS;
	idt_entries[n].always0 = 0;
	idt_entries[n].flags = 0x8E;
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
	idt_ptr_reg.limit = (sizeof(idt_gate_t) * 256) - 1;
	idt_ptr_reg.base = (uint32_t)&idt_entries;

	memset(&idt_entries, 0, sizeof(idt_gate_t) * 256);

	port_byte_out(0x20, 0x11);
	port_byte_out(0xA0, 0x11);

	port_byte_out(0x21, 0x20);
	port_byte_out(0xA1, 0x28);

	port_byte_out(0x21, 0x04);
	port_byte_out(0xA1, 0x02);

	port_byte_out(0x21, 0x01);
	port_byte_out(0xA1, 0x01);

	port_byte_out(0x21, 0x0);
	port_byte_out(0xA1, 0x0);


	set_idt_gate(0, (uint32_t)isr0);
	set_idt_gate(1, (uint32_t)isr1);
	set_idt_gate(2, (uint32_t)isr2);
	set_idt_gate(3, (uint32_t)isr3);
	set_idt_gate(4, (uint32_t)isr4);
	set_idt_gate(5, (uint32_t)isr5);
	set_idt_gate(6, (uint32_t)isr6);
	set_idt_gate(7, (uint32_t)isr7);
	set_idt_gate(8, (uint32_t)isr8);
	set_idt_gate(9, (uint32_t)isr9);
	set_idt_gate(10, (uint32_t)isr10);
	set_idt_gate(11, (uint32_t)isr11);
	set_idt_gate(12, (uint32_t)isr12);
	set_idt_gate(13, (uint32_t)isr13);
	set_idt_gate(14, (uint32_t)isr14);
	set_idt_gate(15, (uint32_t)isr15);
	set_idt_gate(16, (uint32_t)isr16);
	set_idt_gate(17, (uint32_t)isr17);
	set_idt_gate(18, (uint32_t)isr18);
	set_idt_gate(19, (uint32_t)isr19);
	set_idt_gate(20, (uint32_t)isr20);
	set_idt_gate(21, (uint32_t)isr21);
	set_idt_gate(22, (uint32_t)isr22);
	set_idt_gate(23, (uint32_t)isr23);
	set_idt_gate(24, (uint32_t)isr24);
	set_idt_gate(25, (uint32_t)isr25);
	set_idt_gate(26, (uint32_t)isr26);
	set_idt_gate(27, (uint32_t)isr27);
	set_idt_gate(28, (uint32_t)isr28);
	set_idt_gate(29, (uint32_t)isr29);
	set_idt_gate(30, (uint32_t)isr30);
	set_idt_gate(31, (uint32_t)isr31);

	set_idt_gate(32, (uint32_t)irq0);
	set_idt_gate(33, (uint32_t)irq1);
	set_idt_gate(34, (uint32_t)irq2);
	set_idt_gate(35, (uint32_t)irq3);
	set_idt_gate(36, (uint32_t)irq4);
	set_idt_gate(37, (uint32_t)irq5);
	set_idt_gate(38, (uint32_t)irq6);
	set_idt_gate(39, (uint32_t)irq7);
	set_idt_gate(40, (uint32_t)irq8);
	set_idt_gate(41, (uint32_t)irq9);
	set_idt_gate(42, (uint32_t)irq10);
	set_idt_gate(43, (uint32_t)irq11);
	set_idt_gate(44, (uint32_t)irq12);
	set_idt_gate(45, (uint32_t)irq13);
	set_idt_gate(46, (uint32_t)irq14);
	set_idt_gate(47, (uint32_t)irq15);

	set_idt_gate(128, (uint32_t)isr128);
	set_idt_gate(177, (uint32_t)isr177);

	__asm__ __volatile__("lidt %0" : : "m" (idt_ptr_reg));
	__asm__ __volatile__("sti");
}

char *exception_messages[] = {
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint",
	"Into Detected Overflow",
	"Out of Bounds",
	"Invalid Opcode",
	"No Coprocessor",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Bad TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"Unknown Interrupt",
	"Coprocessor Fault",
	"Alignment Check",
	"Machine Check",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
};

/**
 * @brief Interrupt Service Routine (ISR) handler.
 *
 * This function is called when an interrupt is received. It
 * prints the name of the interrupt and halts the system.
 *
 * @param r  The interrupt registers.
 */
void isr_handler(interrupt_regs *r) {
	if (r->int_no < 32) {
		printf("Received interrupt: ");
		printfc(4, "%s", exception_messages[r->int_no]);
		printf("\n");
		printf("System Halted!\n");
		for (;;);
	}

	// TODO: print the interrupt registers
}

void *irq_routines[16] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

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

	if (r->int_no >= 40) {
		port_byte_out(0xA0, 0x20);
	}
	port_byte_out(0x20, 0x20);
}
