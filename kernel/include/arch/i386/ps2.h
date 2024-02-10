#ifndef ARCH_I386_PS2_H
#define ARCH_I386_PS2_H 1

/* PS/2 Controller Driver */

#include <stdint.h>

#define PS2_DATA_PORT				0x60
#define PS2_COMMAND_PORT			0x64

#define PS2_DISABLE_FIRST_PORT		0xAD
#define PS2_ENABLE_FIRST_PORT		0xAE

#define PS2_DISABLE_SECOND_PORT		0xA7
#define PS2_ENABLE_SECOND_PORT		0xA8

#define PS2_READ_CCB				0x20
#define PS2_WRITE_CCB				0x60
#define PS2_TEST_CONTROLLER			0xAA
#define PS2_TEST_FIRST_PORT			0xAB
#define PS2_TEST_SECOND_PORT		0xA9

/**
 * Status Register
 *
 * To get the status register, read from the
 * PS2 command port
 *
 *   7    6     5   4    3    2    1     0
 * |-----------------------------------------|
 * | PE | TOE | U | U | C/D | SF | IBS | OBS |
 * |-----------------------------------------|
 *
 * OBS: output buffer status; 0 = empty, 1 = full
 * IBS: input buffer status; 0 = empty, 1 = full
 * SF: system flag
 * C/D: command/data; 0 = data written to input buffer is data
 * 		for PS/2 device, 1 = data is for the Ps/2 controller
 * U: unknown (chipset specific)
 * TOE: time-out error; 1 = error, 0 = no error
 * PE: parity error; 1 = error, 0 = no error
 */

/**
 * PS/2 Controller Configuration Byte (CCB)
 *
 *   7    6    5     4     3    2    1     0
 * |------------------------------------------|
 * | 0 | FPT | SPC | FPC | 0 | SF | SPI | FPI |
 * |------------------------------------------|
 *
 * FPI: first port interrupt; 1 = enabled, 0 = disabled
 * SPI: second port interrupt; 1 = enabled, 0 = disabled only if
 * 		second PS/2 port supported
 * SF: system flag; 1 = system passed POST, 0 = it didn't
 * FPC: first port clock; 1 = enabled, 0 = disabled
 * SPC: second port clock; 1 = enabled, 0 = disabled
 * FTP: first port translation; 1 = enabled, 0 = disabled
 */
typedef enum {
	PS2_CCB_FPI				= 0x01,
	PS2_CCB_SPI				= 0x02,
	PS2_CCB_FPC				= 0x10,
	PS2_CCB_SPC				= 0x20,
	PS2_CCB_FPT				= 0x40
} PS2_CCB;

/**
 * PS/2 Controller Output Port
 *
 *   7      6     5     4    3     2     1     0
 * |----------------------------------------------|
 * | FPD | FPC | OB2 | OB1 | SPD | SPC | A20 | SR |
 * |----------------------------------------------|
 *
 * SR: system reset (output)
 * A20: A20 gate (output)
 * SPC: second port clock (output)
 * SPD: second port date (output)
 * OB1: oubput buffer full with byte from first port (IRQ1)
 * OB2: output buffer full with byte from second port (IRQ12)
 * FPC: first port clock (output)
 * FPD: first port data (output)
*/

void PS2_init(void);
void PS2_disable_devices(void);
void PS2_enable_devices(int, int);
uint8_t PS2_flush_output_buffer(void);
int PS2_self_test(void);
int PS2_interfaces_test(void);
uint8_t PS2_get_ccb(void);
void PS2_set_ccb(uint8_t);

#endif /* ARCH_I386_PS2_H */
