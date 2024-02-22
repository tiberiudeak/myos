#include <arch/i386/pit.h>
#include <arch/i386/irq.h>
#include <kernel/io.h>
#include <stdio.h>

uint32_t ticks;
uint32_t uptime;

/**
 * @brief Increment ticks
 *
 * This is the IRQ0 handler that increments the ticks every
 * one millisecond and the uptime.
 */
void PIT_IRQ0_handler() {
	ticks++;
	uptime++;
}

/**
 * @brief Initialize the Programmable Interrupt Timer
 *
 * This function first calculates the appropriate divisor to achieve a delay
 * of one millisecond, then configures the PIT and install the handler for IRQ0.
 */
void PIT_init() {

	uint32_t divisor = PIT_FREQ / 1000;

	port_byte_out(COMMAND_PORT, PIT_CW_RL_LSB_MSB | PIT_CW_MODE3);

	// send divisor
	port_byte_out(CH0_DATA_PORT, divisor & 0xFF);
	port_byte_out(CH0_DATA_PORT, (divisor >> 8) & 0xFF);


	ticks = 0;
	uptime = 0;
	irq_install_handler(0, PIT_IRQ0_handler);
}

/**
 * @brief Wait the given milliseconds
 *
 * This function wait for the given milliseconds to pass. The global variable
 * ticks is incremented every millisecond.
 *
 * @param millis The number of milliseconds to wait
 */
void wait_millis(uint16_t millis) {
	ticks = 0;

	while (ticks != millis) __asm__ __volatile__ ("sti; hlt; cli");
}

/**
 * @brief Get uptime since boot in milliseconds
 *
 * This function returns roughly the number of milliseconds tat passed
 * since boot
 */
uint64_t get_uptime() {
	return uptime;
}
