#include <arch/i386/isr.h>
#include <arch/i386/idt.h>
#include <stdio.h>

/**
 * @brief Add the Interrupt Service Routines (ISRs) to the IDT.
 *
 * This function adds the first 32 Interrupt Service Routines (ISRs) to the
 * Interrupt Descriptor Table (IDT). The ISRs are used to handle exceptions
 * and are called when an exception occurs.
 */
void add_isrs_to_idt() {
	set_idt_gate(0, (uint32_t)isr0, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(1, (uint32_t)isr1, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(2, (uint32_t)isr2, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(3, (uint32_t)isr3, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(4, (uint32_t)isr4, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(5, (uint32_t)isr5, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(6, (uint32_t)isr6, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(7, (uint32_t)isr7, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(8, (uint32_t)isr8, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(9, (uint32_t)isr9, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(10, (uint32_t)isr10, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(11, (uint32_t)isr11, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(12, (uint32_t)isr12, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(13, (uint32_t)isr13, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(14, (uint32_t)isr14, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(15, (uint32_t)isr15, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(16, (uint32_t)isr16, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(17, (uint32_t)isr17, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(18, (uint32_t)isr18, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(19, (uint32_t)isr19, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(20, (uint32_t)isr20, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(21, (uint32_t)isr21, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(22, (uint32_t)isr22, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(23, (uint32_t)isr23, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(24, (uint32_t)isr24, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(25, (uint32_t)isr25, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(26, (uint32_t)isr26, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(27, (uint32_t)isr27, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(28, (uint32_t)isr28, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(29, (uint32_t)isr29, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(30, (uint32_t)isr30, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(31, (uint32_t)isr31, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);

	set_idt_gate(128, (uint32_t)isr128, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
	set_idt_gate(177, (uint32_t)isr177, KERNEL_CS, IDT_FLAGS_32BIT_INT | IDT_FLAGS_RING0 | IDT_FLAGS_PRESENT);
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
		printfc(4, "%s\n", exception_messages[r->int_no]);
		printf("cr2: %x ds: %x edi: %x esi: %x\n", r->cr2, r->ds, r->edi, r->esi);
		printf("ebp: %x esp: %x ebx: %x edx: %x\n", r->ebp, r->esp, r->ebx, r->edx);
		printf("ecx: %x eax: %x int_no: %x err_code: %x\n", r->ecx, r->eax, r->int_no, r->err_code);
		printf("eip: %x cs: %x eflags: %x useresp: %x ss: %x\n", r->eip, r->cs, r->eflags, r->useresp, r->ss);
		printf("Kernel Panic - System Halted!\n");
		for (;;);
	}
}
