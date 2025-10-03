#define pr_log_fmt(msg) "ACPI: %s: " msg, __func__
#include <kernel/acpi.h>
#include <kernel/string.h>
#include <kernel/tty.h>

#include <stdint.h>


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
int acpi_compute_checksum(struct acpi_sdt_header *table_header) {
	unsigned char sum = 0;

	for (size_t i = 0; i < table_header->length; i++) {
		sum += ((char *) table_header)[i];
	}

	return sum;
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
int acpi_validate_rsdp(struct acpi_rsdp_descriptor *rsdp) {
	uint8_t checksum = 0;

	for (size_t i = 0; i < sizeof(struct acpi_rsdp_descriptor); i++) {
		checksum += ((char *) rsdp)[i];
	}

	if (checksum != 0) {
		pr_log("RSDP checksum is invalid\n");
		return 1;
	}

	return 0;
}

/**
 * @brief Detect the Root System Description Pointer (RSDP) in memory.
 *
 * This function searches for the RSDP signature in the memory
 * range 0x00080000 to 0x00081024 and 0x000E0000 to 0x000FFFFF.
 *
 * @return The address of the RSDP if found, NULL otherwise.
 */
void *acpi_find_rsdp() {
	char *start = (char *) 0x00080000;
	char *end = (char *) 0x00081024;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			if (acpi_validate_rsdp((struct acpi_rsdp_descriptor *) start) == 0) {
				return start;
			}
		}
		start += 16;
	}

	start = (char *) 0x000E0000;
	end = (char *) 0x000FFFFF;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			if (acpi_validate_rsdp((struct acpi_rsdp_descriptor *) start) == 0) {
				return start;
			}
		}
		start += 16;
	}

	return NULL;
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
	struct acpi_rsdt *rsdt = (struct acpi_rsdt *) RSDT_pointer;
	int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

	for (int i = 0; i < entries; i++) {
		struct acpi_sdt_header *h =
			(struct acpi_sdt_header *) rsdt->pointer_to_other_sdt[i];

		if (memcmp(h->signature, "FACP", 4) == 0) {
			if (acpi_compute_checksum(h) == 0) {
				return (void *) h;
			} else {
				return NULL;
			}
		}
	}

	return NULL;
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
uint8_t acpi_init() {
	pr_log("Initializing ACPI\n");
	struct acpi_rsdp_descriptor *rsdp = acpi_find_rsdp();

	if (rsdp == NULL) {
		pr_log("RSDP not found!\n");
		return 1;
	} else {
		pr_log("RSDP found at physical addr: 0x%x\n", rsdp);
	}

	struct FADT *fadt = (struct FADT *) find_FACP((void *) rsdp->rsdt_phy_address);

	if (fadt == NULL) {
		pr_log("FADT not found!\n");
		return 1;
	} else {
		pr_log("FADT found at physical addr: 0x%x\n", fadt);
	}

	return 0;
}
