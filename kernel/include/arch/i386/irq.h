#ifndef ARCH_I386_IRQ_H
#define ARCH_I386_IRQ_H 1

#include <arch/i386/isr.h>

void irq_handler(interrupt_regs *r);
void irq_install_handler(int irq, void (*handler)(interrupt_regs *r));
void irq_uninstall_handler(int irq);
void add_irqs_to_idt(void);

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#endif /* ARCH_I386_IRQ_H */
