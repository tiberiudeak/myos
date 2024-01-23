#include <stdint.h>
#include <kernel/io_port.h>

/**
 * @brief Read a byte from a specified I/O port.
 *
 * This function reads a byte from the specified I/O port using
 * the IN instruciton.
 *
 * @param port  The 16-bit I/O port number from which to read the byte.
 *
 * @return The byte read from the specified I/O port.
 */
uint8_t port_byte_in(uint16_t port) {
	uint8_t result;
	__asm__("in %%dx, %%al" : "=a" (result) : "d" (port));

	return result;
}

/**
 * @brief Write a byte to a specified I/O port.
 *
 * This function writes a byte to the specified I/O port using
 * the OUT instruction.
 *
 * @param port  The 16-bit I/O port number to which to write the byte.
 */
void port_byte_out(uint16_t port, uint8_t data) {
	__asm__("out %%al, %%dx" : : "a" (data), "d" (port));
}

/**
 * @brief Read a word from a specified I/O port.
 *
 * This function reads a word (16 bits) from the specified I/O port
 * using the IN instruction.
 *
 * @param port  The 16-bit I/O port number from which to read the word.
 *
 * @return The word read from the specified I/O port.
 */
uint16_t port_word_in(uint16_t port) {
	uint16_t result;
	__asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));

	return result;
}

/**
 * @brief Write a word to a specified I/O port.
 *
 * This function writes a word (16 bits) to the specified I/O port
 * using the OUT instruction.
 *
 * @param port  The 16-bit I/O port number to which to write the word.
 */
void port_word_out(uint16_t port, uint16_t data) {
	__asm__("out %%ax, %%dx" : : "a" (data), "d" (port));
}
