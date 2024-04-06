#include <arch/i386/ps2.h>
#include <kernel/io.h>
#include <kernel/tty.h>

int PS2_dual_port;

/**
 * @brief Initialize the PS/2 Controller
 *
 * This function performs the initialization steps for the PS/2
 * Controller. The ports are disabled first, then a self check test
 * is performed, followed by a test for both interfaces. The available
 * ports are enabled at the end.
 */
int PS2_init() {
	printk("Initializing PS2 Controller\n");
	int ret;

	// disable device
	PS2_disable_devices();

	// flush the output buffer
	PS2_flush_output_buffer();

	// perform controller self test
	ret = PS2_self_test();

	if (ret) {
		printk("\tPS/2 self test passed\n");
	}
	else {
		printk("\tPS/2 self test failed\n");
		return 1;
	}

	// perform interface tests
	ret = PS2_interfaces_test();

	if (ret == 0) {
		printk("\tNo PS/2 interfaces available\n");
		return 1;
	}

	if (ret == 3) {
		printk("\tPS/2 available interfaces: 2\n");
		PS2_enable_devices(1, 1);
		return 0;
	}

	if (ret == 2) {
		printk("\tOnly second PS/2 interface available\n");
		PS2_enable_devices(0, 1);
		return 0;
	}

	printk("\tOnly first PS/2 interface available\n");
	PS2_enable_devices(1, 0);
	return 0;
}

/**
 * @brief Get PS/2 status register.
 *
 * @return PS/2  status register
 */
uint8_t __PS2_get_status_reg() {
	return port_byte_in(PS2_COMMAND_PORT);
}

/**
 * @brief Wait until response is available from PS/2
 *
 * This function waits until a response is available from the
 * PS/2 Controller (a response is available when bit 0 from the
 * status register is set to 1)
 */
void __wait_for_response() {
	uint8_t status_reg = __PS2_get_status_reg();

	while ((status_reg & 0b00000001) == 0) {
		status_reg = __PS2_get_status_reg();
		io_wait();
	}
}

/**
 * @brief Wait until PS/2 Controller can accept the next byte
 *
 * This function waits until the input buffer of the PS/2 Controller
 * is empty (it is empty then bit 1 of the status register is clear)
*/
void __wait_to_send_second_byte() {
	uint8_t status_reg = __PS2_get_status_reg();

	while ((status_reg & 0b0000010) == 2) {
		status_reg = __PS2_get_status_reg();
		io_wait();
	}
}

/**
 * @brief Disable PS/2 ports
 *
 * This function will disable the first and second port of the PS/2
 * controller.
 */
void PS2_disable_devices() {
	port_byte_out(PS2_COMMAND_PORT, PS2_DISABLE_FIRST_PORT);
	io_wait();

	port_byte_out(PS2_COMMAND_PORT, PS2_DISABLE_SECOND_PORT);
	io_wait();
}

/**
 * @brief Enable PS/2 ports
 *
 * This function enables the first PS/2 port if port1 is 1 and
 * the second port if port2 is 1. Interrupts are also enabled for
 * both ports.
 *
 * @param port1 1 if port1 should be enabled, 0 otherwise
 * @param port2 1 if port2 should be enabled, 0 otherwise
 */
void PS2_enable_devices(int port1, int port2) {

	if (port1 || port2) {
		uint8_t ccb = PS2_get_ccb();

		ccb |= 0b00000011;
		PS2_set_ccb(ccb);
	}

	if (port1) {
		port_byte_out(PS2_COMMAND_PORT, PS2_ENABLE_FIRST_PORT);
		io_wait();
	}

	if (port2) {
		port_byte_out(PS2_COMMAND_PORT, PS2_ENABLE_SECOND_PORT);
		io_wait();
	}
}

/**
 * @brief Read PS/2 Controller output buffer
 *
 * This function reads and returns the byte in the
 * PS/2 Controller output buffer
 *
 * @return the byte in the PS/2 output buffer
 */
uint8_t PS2_flush_output_buffer() {
	return port_byte_in(PS2_DATA_PORT);
}

/**
 * @brief Test the PS/2 Controller
 *
 * This function performs a self test on the PS/2 Controller.
 *
 * @return 1 if test passed, 0 otherwise
 */
int PS2_self_test() {
	uint8_t ccb = PS2_get_ccb();

	if (ccb & PS2_CCB_SPC) {
		PS2_dual_port = 1;
	}

	port_byte_out(PS2_COMMAND_PORT, PS2_TEST_CONTROLLER);

	__wait_for_response();

	uint8_t response = port_byte_in(PS2_DATA_PORT);

	if (response == 0x55) {
		port_byte_out(PS2_COMMAND_PORT, PS2_WRITE_CCB);

		__wait_to_send_second_byte();

		port_byte_out(PS2_COMMAND_PORT, ccb);

		return 1;
	}

	return 0;
}

/**
 * @brief Test the two PS/2 interfaces
 *
 * This function tests both ports of the PS/2 Controller.
 *
 * @return 3 if both ports are available, 2 if only the second port is,
 * 		   1 if the first port is, 0 if no port is available
 */
int PS2_interfaces_test() {
	port_byte_out(PS2_COMMAND_PORT, PS2_TEST_FIRST_PORT);

	__wait_for_response();

	uint8_t response1 = port_byte_in(PS2_DATA_PORT);

	if (response1 != 0) {
		printk("PS/2 port 1 test failed\n");
	}

	port_byte_out(PS2_COMMAND_PORT, PS2_TEST_SECOND_PORT);

	__wait_for_response();

	uint8_t response2 = port_byte_in(PS2_DATA_PORT);

	if (response2 != 0) {
		printk("PS/2 port 2 test failed\n");
	}

	if (response1 == 0 && response2 == 0) {
		return 3;
	}

	if (response1 == 0) {
		return 1;
	}

	if (response2 == 0) {
		return 2;
	}

	return 0;
}

/**
 * @brief Read PS/2 Controller Configuration Byte
 *
 * This function reads and returns the PS/2 CCB.
 *
 * @return PS/2 Controller Configuration Byte
 */
uint8_t PS2_get_ccb() {
	port_byte_out(PS2_COMMAND_PORT, PS2_READ_CCB);
	io_wait();

	__wait_for_response();

	return port_byte_in(PS2_DATA_PORT);
}

/**
 * @brief Set PS/2 Controller Configuration Byte
 *
 * This function writes the PS/2 CCB with the given value.
 *
 * @param conf The configuration byte
 */
void PS2_set_ccb(uint8_t conf) {
	port_byte_out(PS2_COMMAND_PORT, PS2_WRITE_CCB);

	__wait_to_send_second_byte();

	port_byte_out(PS2_COMMAND_PORT, conf);
}
