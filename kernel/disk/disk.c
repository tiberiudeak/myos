#include <disk/disk.h>
#include <kernel/io.h>
#include <stddef.h>

/**
 * @brief Read sector from disk into main memory
 *
 * This function reads the indicated number of sectors starting with the
 * given sector and writes the data into the main mamory at the given
 * address.
 *
 * @param starting_sector 	The starting sector from which to read
 * @param size				The number of sectors to read
 * @param addr				The location in main memory to store the data
 *
 * @return Error code or 0 if successful
 */
uint8_t read_sectors(uint32_t starting_sector, uint32_t size, uint32_t addr) {

	port_byte_out(ATA_PIO_PR_SCR, size);
	port_byte_out(ATA_PIO_PR_SNR, starting_sector & 0xFF);
	port_byte_out(ATA_PIO_PR_CLR, ((starting_sector >> 8) & 0xFF));
	port_byte_out(ATA_PIO_PR_CHR, ((starting_sector >> 16) & 0xFF));
	port_byte_out(ATA_PIO_PR_DHR, ((starting_sector >> 24) & 0x0F));
	port_byte_out(ATA_PIO_PR_SR, READ_WITH_RETRY);

	// address pointer to write to
	uint16_t *addr_ptr = (uint16_t*) addr;

	// read and write to RAM one sector at a time
	for (size_t i = size; i > 0; i--) {
		// check if BSY bit is set, in which case wait
		while (port_byte_in(ATA_PIO_PR_SR) & ATA_PIO_SR_BSY) ;

		// get two bytes at a time
		for (uint32_t j = 0; j < 256; j++) {
			*addr_ptr = port_word_in(ATA_PIO_PR_DR);
			addr_ptr++;
		}

		// wait recommended 400ns
		for (uint8_t k = 0; k < 4; k++) {
			port_byte_in(0x3F6);
		}
	}

	// check for errors
	while (port_byte_in(ATA_PIO_PR_SR) & (1 << 7)) ;

	uint8_t errors = 0;
	errors = port_byte_in(ATA_PIO_PR_ER);

	return errors;
}


/**
 * @brief Write sectors from main memory on the disk
 *
 * This function writes the indicated number of sectors starting with the
 * given sector and writes the data into the main mamory at the given
 * address.
 *
 * @param starting_sector 	The starting sector from which to write
 * @param size				The number of sectors to write
 * @param addr				The location in main memory from where to take the data
 *
 * @return Error code or 0 if successful
 */
uint8_t write_sectors(uint32_t starting_sector, uint32_t size, uint32_t addr) {

	port_byte_out(ATA_PIO_PR_SCR, size);
	port_byte_out(ATA_PIO_PR_SNR, starting_sector & 0xFF);
	port_byte_out(ATA_PIO_PR_CLR, ((starting_sector >> 8) & 0xFF));
	port_byte_out(ATA_PIO_PR_CHR, ((starting_sector >> 16) & 0xFF));
	port_byte_out(ATA_PIO_PR_DHR, ((starting_sector >> 24) & 0x0F));
	port_byte_out(ATA_PIO_PR_SR, WRITE_WITH_RETRY);

	// address pointer to write to
	uint16_t *addr_ptr = (uint16_t*) addr;

	for (size_t i = size; i > 0; i--) {
		// check if BSY bit is set, in which case wait
		while (port_byte_in(ATA_PIO_PR_SR) & ATA_PIO_SR_BSY) ;

		for (uint32_t j = 0; j < 256; j++) {
            port_byte_out(ATA_PIO_PR_DR, *addr_ptr);
            addr_ptr++;
		}

		// wait recommended 400ns
		for (uint8_t k = 0; k < 4; k++) {
			port_byte_in(0x3F6);
		}
	}

    // cache flush command
    port_byte_out(ATA_PIO_PR_SR, 0xE7);
    while (port_byte_in(ATA_PIO_PR_SR) & ATA_PIO_SR_BSY) {}

	// check for errors
	while (port_byte_in(ATA_PIO_PR_SR) & (1 << 7)) {}

	uint8_t errors = 0;
	errors = port_byte_in(ATA_PIO_PR_ER);

	return errors;
}
