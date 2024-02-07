#ifndef ARCH_I386_PIC_H
#define ARCH_I386_PIC_H 1

#include <stdint.h>

#define PIC1 				0x20		// master PIC
#define PIC2 				0xA0		// slave PIC

#define PIC1_COMMAND_PORT	PIC1
#define PIC1_DATA_PORT		(PIC1 + 1)

#define PIC2_COMMAND_PORT	PIC2
#define PIC2_DATA_PORT		(PIC2 + 1)

#define PIC_EOI				0x20		// end of interrupt

#define PIC_READ_IRR		0x0a
#define PIC_READ_ISR		0x0b

/**
 * PIC Initialization Command Word 1
 *
 *   A0  D7   D6   D5   D4   D3    D2     D1    D0
 * |---|--------------------------------------------|
 * | 0 | A7 | A6 | A5 | 1 | LTIM | ADI | SNGL | IC4 |
 * |---|--------------------------------------------|
 *
 * IC4: 1 if ICW4 is needed, 0 otherwise
 * SNGL: 1 = single PIC, 0 = cascade mode and ICW3 must be also sent
 * ADI: call address interval; 1 = interval 4, 0 = interval 8
 * LTIM: 1 = level triggered mode, 0 = edge triggered mode
 * A7-A5: for MCS80/85 mode only
 * D4: set to 1: initialize PIC
 *
 */
typedef enum {
	PIC_ICW1_ICW4				= 0x01,
	PIC_ICW1_SINGLE_MODE		= 0x02,
	PIC_ICW1_CASCADE_MODE		= 0x00,
	PIC_ICW1_INTERVAL4			= 0x04,
	PIC_ICW1_INTERVAL8			= 0x00,
	PIC_ICW1_LEVEL_MODE			= 0x08,
	PIC_ICW1_EDGE_MODE			= 0x00,
	PIC_ICW1_INITIALIZE			= 0x10
} PIC_ICW1;

/**
 * PIC Initialization Command Word 2
 *
 *  A0   D7   D6   D5   D4   D3   D2    D1   D0
 * |---|----------------------------------------|
 * | 1 | T7 | T6 | T5 | T4 | T3 | A10 | A9 | A8 |
 * |---|----------------------------------------|
 *
 * In an 8086 system, only the T7-T3 bits are used: they are inserted
 * in the most significant bits of the vectoring byte and the 8259A sets
 * the three remaining less significant bits according to the interrupt level
 */

/**
 * PIC Initialization Command Word 3
 *
 * PIC1
 *
 *  A0   D7   D6   D5   D4   D3   D2   D1   D0
 * |---|---------------------------------------|
 * | 1 | S7 | S6 | S5 | S4 | S3 | S2 | S1 | S0 |
 * |---|---------------------------------------|
 *
 * 1 = IR input has a slave, 0 it doesn't
 * For example, if the slave is connected to the IR2 pin on the
 * PIC1 => 1|00000100 = 0x04
 *
 * PIC2
 *
 *  A0  D7  D6  D5  D4  D3   D2    D1    D0
 * |---|-------------------------------------|
 * | 1 | 0 | 0 | 0 | 0 | 0 | Id2 | Id1 | Id0 |
 * |---|-------------------------------------|
 *
 * Slave ID is equal to the corresponding master IR input
 * For example, if the slave is connected to the IR2 pin on the
 * PIC1 => 1|00000010 = 0x02
 */

/**
 * PIC Initialization Command Word 4
 *
 *  A0   D7  D6  D5   D4    D3    D2    D1     D0
 * |---|-------------------------------------------|
 * | 1 | 0 | 0 | 0 | SFNM | BUF | M/S | AEOI | MPM |
 * |---|-------------------------------------------|
 *
 * MPM: 1 = 8086/8088 mode; 0 = MCS80/85 mode
 * AEOI: 1 = automatic EOI; 0 = normal EOI
 * BUF&M/S: buffered mode
 * SFNM: 1 = special fully nestet mode; 0 = not SFNM
 */
typedef enum {
	PIC_ICW4_8086_MODE			= 0x01,
	PIC_ICW4_AEOI				= 0x02,
	PIC_ICW4_SFNM				= 0x10,
	PIC_ICW4_MASTER_BUFFERED	= 0x0C,
	PIC_ICW4_SLAVE_BUFFERED		= 0x08
} PIC_ICW4;

void PIC_send_EOI(uint8_t);
void PIC_disable(void);
void PIC_configure(uint8_t, uint8_t);
void IRQ_set_mask(uint8_t);
void IRQ_clear_mask(uint8_t);
uint16_t PIC_get_IRR(void);
uint16_t PIC_get_ISR(void);

#endif
