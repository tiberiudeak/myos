#include <arch/i386/pic.h>
#include <kernel/io.h>

#include <stdint.h>

/**
 * @brief Send EOI (End of Interrupt) command to the PIC
 *
 * This function sends the EOI command to the PIC to notify
 * that the interrupt has been handled.
 *
 * @param irq  The IRQ number
 */
void PIC_send_EOI(uint8_t irq) {
	if (irq >= 8) {
		port_byte_out(PIC2_COMMAND_PORT, PIC_EOI);
	}

	port_byte_out(PIC1_COMMAND_PORT, PIC_EOI);
}

/**
 * @brief Disable the PIC
 *
 * This function disables the PIC by setting the masks
 * of both PICs to 0xFF (all interrupts disabled).
 */
void PIC_disable(void) {
	port_byte_out(PIC1_DATA_PORT, 0xFF);
	port_byte_out(PIC2_DATA_PORT, 0xFF);
}

/**
 * @brief Set the mask for the specified IRQ line
 *
 * This function sets the mask for the specified IRQ line
 * by setting the corresponding bit in the PIC's data port.
 *
 * @param irq_line  The IRQ line
 */
void IRQ_set_mask(uint8_t irq_line) {
	uint16_t port;
	uint8_t value;

	if (irq_line < 8) {
		port = PIC1_DATA_PORT;
	} else {
		port = PIC2_DATA_PORT;
		irq_line -= 8;
	}

	value = port_byte_in(port) | (1 << irq_line);
	port_byte_out(port, value);
}

/**
 * @brief Clear the mask for the specified IRQ line
 *
 * This function clears the mask for the specified IRQ line
 * by clearing the corresponding bit in the PIC's data port.
 *
 * @param irq_line  The IRQ line
 */
void IRQ_clear_mask(uint8_t irq_line) {
	uint16_t port;
	uint8_t value;

	if (irq_line < 8) {
		port = PIC1_DATA_PORT;
	} else {
		port = PIC2_DATA_PORT;
		irq_line -= 8;
	}

	value = port_byte_in(port) & ~(1 << irq_line);
	port_byte_out(port, value);
}

/**
 * @brief Configure the PIC
 *
 * This function configures the PIC by sending the
 * Initialization Command Words (ICWs) to the PICs.
 *
 * @param offset1  The offset for the master PIC
 * @param offset2  The offset for the slave PIC
 */
void PIC_configure(uint8_t offset1, uint8_t offset2) {
	uint8_t a1, a2;

	// save masks
	a1 = port_byte_in(PIC1_DATA_PORT);
	a2 = port_byte_in(PIC2_DATA_PORT);

	// PIC_ICW1
	port_byte_out(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
	io_wait();
	port_byte_out(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
	io_wait();

	// PIC ICW2
	port_byte_out(PIC1_DATA_PORT, offset1);
	io_wait();
	port_byte_out(PIC2_DATA_PORT, offset2);
	io_wait();

	// PIC ICW3
	port_byte_out(PIC1_DATA_PORT, 0x04);
	io_wait();
	port_byte_out(PIC2_DATA_PORT, 0x02);
	io_wait();

	// PIC ICW4
	port_byte_out(PIC1_DATA_PORT, PIC_ICW4_8086_MODE);
	io_wait();
	port_byte_out(PIC2_DATA_PORT, PIC_ICW4_8086_MODE);
	io_wait();

	port_byte_out(PIC1_DATA_PORT, a1);
	io_wait();
	port_byte_out(PIC2_DATA_PORT, a2);
	io_wait();
}

/**
 * @brief Send command to the PICs and return the results
 *
 * This function sends a command to the PICs and returns the
 * results (the upper byte is for the slave PIC).
 *
 * @param ocw3  The command to send
 */
static uint16_t __PIC_get_irq_reg(uint8_t ocw3) {
	port_byte_out(PIC1_COMMAND_PORT, ocw3);
	port_byte_out(PIC2_COMMAND_PORT, ocw3);

	return (port_byte_in(PIC2_COMMAND_PORT << 8) |
			port_byte_in(PIC1_COMMAND_PORT));
}

/**
 * @brief Get the In-Service Register (ISR)
 *
 * This function gets the In-Service Register (ISR) from the PIC.
 *
 * @return The ISRs for both PICs (the upper byte is for the slave PIC)
 */
uint16_t PIC_geT_IRR(void) {
	return __PIC_get_irq_reg(PIC_READ_IRR);
}

/**
 * @brief Get the Interrupt Request Register (IRR)
 *
 * This function gets the Interrupt Request Register (IRR) from the PIC.
 *
 * @return The IRRs for both PICs (the upper byte is for the slave PIC)
 */
uint16_t PIC_get_ISR(void) {
	return __PIC_get_irq_reg(PIC_READ_ISR);
}
