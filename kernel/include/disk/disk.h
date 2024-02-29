#ifndef DISK_H
#define DISK_H

#include <stdint.h>

typedef enum {
	READ_WITH_RETRY				= 0x20,
	WRITE_WITH_RETRY			= 0x30
} ATA_PIO_COMMANDS;

#define ATA_PIO_PR_BASE			0x1F0
#define ATA_PIO_PR_CTRL_BASE	0x3F7

#define ATA_PIO_SEC_BASE		0x170
#define ATA_PIO_SEC_CTRL_BASE	0x376

#define ATA_PIO_PR_DR			(ATA_PIO_PR_BASE + 0)	// data reg
#define ATA_PIO_PR_ER			(ATA_PIO_PR_BASE + 1)	// error reg
#define ATA_PIO_PR_SCR			(ATA_PIO_PR_BASE + 2)	// sector count reg
#define ATA_PIO_PR_SNR			(ATA_PIO_PR_BASE + 3)	// sector number reg
#define ATA_PIO_PR_CLR			(ATA_PIO_PR_BASE + 4)	// cylinder low reg
#define ATA_PIO_PR_CHR			(ATA_PIO_PR_BASE + 5)	// cylinder high
#define ATA_PIO_PR_DHR			(ATA_PIO_PR_BASE + 6)	// drive/head reg
#define ATA_PIO_PR_SR			(ATA_PIO_PR_BASE + 7)	// status/command reg

/**
 * Error Register Layout
 *
 *   7      6     5    4      3     2       1       0
 * |---------------------------------------------------|
 * | BBK | UNC | MC | IDNF | MCR | ABRT | TKZNF | AMNF |
 * |---------------------------------------------------|
 *
 * AMNF: Address mark not found
 * TKZNF: Track zero not found
 * ABRT: Aborted command
 * MCR: Media change request
 * IDNF: ID not found
 * MC: Media changed
 * UNC: Uncorrectable data error
 * BBK: Bad Block detected
 */

/**
 * Status Register Layout
 *
 *   7      6    5     4     3     2      1     0
 * |-----------------------------------------------|
 * | BSY | RDY | DF | SRV | DRQ | CORR | IDX | ERR |
 * |-----------------------------------------------|
 *
 * ERR: Indicates an error occured
 * IDX: index - always set to 0
 * CORR: Corrected data
 * DRQ: set when the drive has PIO data to transfer
 * SRV: Overlapped mode service request
 * DF: Drive Fault Error
 * RDY: bit is clear when drive is spun down or after an error
 * BSY: the drive is preparing to send/receive data
 */
typedef enum {
	ATA_PIO_SR_BSY			= 0x80
} ATA_PIO_STATUS_REG;

uint8_t read_sectors(uint8_t, uint8_t, uint32_t);

#endif /* !DISK_H */
