#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/acpi.h>

/**
 * @brief Detects the Root System Description Pointer (RSDP) in memory.
 *
 * This function searches for the RSDP signature in the memory
 * range 0x00080000 to 0x00081024 and 0x000E0000 to 0x000FFFFF.
 *
 * @return The address of the RSDP if found, 0 otherwise.
 */
uint32_t RSDP_detect() {
	char *start = (char *)0x00080000;
	char *end = (char *)0x00081024;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			return (uint32_t)start;
		}
		start += 16;
	}

	start = (char *)0x000E0000;
	end = (char *)0x000FFFFF;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			return (uint32_t)start;
		}
		start += 16;
	}

	return 0;
}

/**
 * @brief Validates the Root System Description Pointer (RSDP).
 *
 * This function validates the RSDP by checking the checksum of the
 * RSDP structure.
 *
 * @param rsdp The address of the RSDP.
 *
 * @return 1 if the RSDP is valid, 0 otherwise.
 */
int RSDP_validate(uint32_t rsdp) {
	RSDP_descriptor_t *rsdp_desc = (RSDP_descriptor_t *)rsdp;

	uint8_t checksum = 0;

	for (size_t i = 0; i < sizeof(RSDP_descriptor_t); i++) {
		checksum += ((char *)rsdp_desc)[i];
	}

	if (checksum != 0) {
		printf("RSDP checksum is invalid\n");
		return 1;
	}

	return 0;
}

void ACPI_init() {
	uint32_t rsdp = RSDP_detect();

	if (rsdp == 0) {
		printf("RSDP not found\n");
		return;
	}

	if (RSDP_validate(rsdp)) {
		printf("RSDP is invalid\n");
		return;
	}
}
