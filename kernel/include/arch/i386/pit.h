#ifndef ARCH_I386_PIT_H
#define ARCH_I386_PIT_H 1

#include <arch/i386/isr.h>
#include <stdint.h>

#define CH0_DATA_PORT		0x40
#define CH1_DATA_PORT		0x41
#define CH2_DATA_PORT		0x43
#define COMMAND_PORT		0x43

#define PIT_FREQ			1193182

/**
 * PIT Command Byte layout
 *
 *   D7    D6     D5    D4   D3   D2   D1   D0
 * |--------------------------------------------|
 * | SC1 | SC0 | RL1 | RL0 | M2 | M1 | M0 | BCD |
 * |--------------------------------------------|
 *
 * SC: select counter; SC1 SC0: 00 = select counter 1, 01 = counter 1,
 * 		10 = counter 2, 11 = read-back command (8258 chip only)
 * RL: read/load; RL1 RL0: 00 = latch count value command, 01 = lobyte only
 * 		access mode, 10 = hibyte only access mode, 11 = lobyte/hibyte access mode
 * M: mode; M2 M1 M0: 000 = mode 0, 001 = mode 1, X10 = mode 2, X11 = code 3,
 * 		100 = mode 4, 101 = mode 5
 * BCD: 0 = binary counter 16 bits, 1 = binary coded decimal counter
 */
typedef enum {
	PIT_CW_COUNTER1				= 0x40,
	PIT_CW_COUNTER2				= 0x80,
	PIT_CW_RL_MSB				= 0x20,
	PIT_CW_RL_LSB				= 0x10,
	PIT_CW_RL_LSB_MSB			= 0x30,
	PIT_CW_MODE1				= 0x02,
	PIT_CW_MODE2				= 0x04,
	PIT_CW_MODE3				= 0x06,
	PIT_CW_MODE4				= 0x08,
	PIT_CW_MODE5				= 0x0A,
	PIT_CW_BCD					= 0x01
} PIT_CW;

void PIT_init(void);
void PIT_IRQ0_handler(struct interrupt_regs *);
void wait_millis(uint16_t);
uint64_t get_uptime(void);
uint32_t random(void);

extern void irq0();

#endif
