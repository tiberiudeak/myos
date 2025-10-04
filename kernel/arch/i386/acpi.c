#define pr_log_fmt(msg) "ACPI: " msg
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

void acpi_print_table_header(void *physical_address) {
	if (memcmp(((struct acpi_rsdp_descriptor *) physical_address)->signature,
				"RSD PTR ", 8) == 0) {
		pr_log("RSDP at 0x%8x (v%d %.6s)\n",
				physical_address,
				((struct acpi_rsdp_descriptor *) physical_address)->revision,
				//== 0 ? 1 : 2,
				((struct acpi_rsdp_descriptor *) physical_address)->oem_id);
	} else {
		struct acpi_sdt_header *header = (struct acpi_sdt_header *) physical_address;

		pr_log("%.4s at 0x%8x (v%d %.6s %.8s)\n",
				header->signature,
				physical_address,
				header->revision,// == 0 ? 1 : 2,
				header->oem_id,
				header->oem_table_id);
	}
}

uint8_t acpi_parse_root_table(struct acpi_rsdp_descriptor *rsdp) {
	struct acpi_rsdt *rsdt;
	struct acpi_sdt_header *table;
	uint32_t nr_entries = 0;

	if (rsdp->revision > 0) {
		pr_log("ACPI version 2, using XSDT table not implemented!\n");
		return 1;
	}

	rsdt = (struct acpi_rsdt *) rsdp->rsdt_phy_address;

	if (!rsdt) {
		pr_log("Invalid RSDT physical address!\n");
		return 1;
	}

	// validate table length
	if (rsdt->header.length < (sizeof(struct acpi_sdt_header) + sizeof(uint32_t))) {
		pr_log("Invalid RSDT table length!\n");
		return 1;
	}

	// check checksum
	if (acpi_compute_checksum(&rsdt->header) != 0) {
		pr_log("Invalid RSDT checksum!\n");
		return 1;
	}

	acpi_print_table_header(rsdt);

	// determine nr of entries
	nr_entries = (rsdt->header.length - sizeof(struct acpi_sdt_header)) / sizeof(uint32_t);

	for (uint32_t i = 0; i < nr_entries; i++) {
		table = (struct acpi_sdt_header *) rsdt->pointer_to_other_sdt[i];

		if (table && acpi_compute_checksum(table) == 0) {
			acpi_print_table_header(table);

			// add table in global list
		} else {
			pr_log("%.4s: wrong checksum!\n", table->signature);
		}
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
uint8_t acpi_init() {
	pr_log("Initializing ACPI\n");
	struct acpi_rsdp_descriptor *rsdp = acpi_find_rsdp();

	if (rsdp == NULL) {
		pr_log("RSDP not found!\n");
		return 1;
	} else {
		acpi_print_table_header(rsdp);
	}

	if (acpi_parse_root_table(rsdp)) {
		return 1;
	}

	return 0;
}
