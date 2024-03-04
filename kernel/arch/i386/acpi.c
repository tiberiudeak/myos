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
 * @return The address of the RSDP if found, NULL otherwise.
 */
void *RSDP_detect() {
	char *start = (char *)0x00080000;
	char *end = (char *)0x00081024;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			return start;
		}
		start += 16;
	}

	start = (char *)0x000E0000;
	end = (char *)0x000FFFFF;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			return start;
		}
		start += 16;
	}

	return NULL;
}

/**
 * @brief Validates the Root System Description Pointer (RSDP).
 *
 * This function validates the RSDP by checking the checksum of the
 * RSDP structure.
 *
 * @param rsdp The address of the RSDP.
 *
 * @return 0 if the RSDP is valid, 1 otherwise.
 */
int RSDP_validate(RSDP_descriptor_t *rsdp) {
	uint8_t checksum = 0;

	for (size_t i = 0; i < sizeof(RSDP_descriptor_t); i++) {
		checksum += ((char *)rsdp)[i];
	}

	if (checksum != 0) {
		printf("RSDP checksum is invalid\n");
		return 1;
	}

	return 0;
}

/**
 * @brief Discovers location of ACPI Tables
 *
 * This function discovers the location of the present ACPI tables
 * and initializes the global variables with their addresses.
 *
 * TODO: create and populate global addresses
 * TODO: create a function  that displays the hardware information
 */
void ACPI_init() {
	printf("Detecting ACPI");
	RSDP_descriptor_t *rsdp = RSDP_detect();

	if (rsdp == NULL) {
		printfc(4, "\t\t\tRSDP not found\n");
		return;
	}

	if (RSDP_validate(rsdp)) {
		printfc(4, "\t\t\tRSDP is invalid\n");
		return;
	}

	FADT *fadt = (FADT*) find_FACP((void *)rsdp->RsdtAddress);

	if (fadt == NULL) {
		printfc(4, "\t\t\tFADT not found!\n");
		return;
	}

	printfc(2, "\t\t\tdone\n");
}

/**
 * @brief Compute and return checksum of the given SDT Header
 *
 * This function receives a System Descriptor Table Header and
 * returns its checksum.
 *
 * @param table_header ACPI SDT Header
 *
 * @return the checksum
 */
int ACPI_do_checksum(ACPISDT_header *table_header) {
	unsigned char sum = 0;

	for (size_t i = 0; i < table_header->Length; i++) {
		sum += ((char *) table_header)[i];
	}

	return sum;
}

/**
 * @brief Searches for the Fixed ACPI Description Table
 *
 * This function searches for the FADT signature in the System Descriptor
 * Tables present in the Root System Descriptor Table.
 *
 * @param RSDT_pointer address of the RSDT
 */
void *find_FACP(void *RSDT_pointer) {
	RSDT *rsdt = (RSDT*) RSDT_pointer;
	int entries = (rsdt->header.Length - sizeof(rsdt->header)) / 4;

	for (int i = 0; i < entries; i++) {
		ACPISDT_header *h = (ACPISDT_header*) rsdt->pointer_to_other_SDT[i];

		if (memcmp(h->Signature, "FACP", 4) == 0) {
			if (ACPI_do_checksum(h) == 0) {
				return (void*) h;
			}
			else {
				return NULL;
			}
		}
	}

	return NULL;
}
