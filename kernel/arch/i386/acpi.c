#define pr_log_fmt(msg) "ACPI: " msg
#include <kernel/acpi.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <kernel/utils.h>
#include <mm/vmm.h>

#include <stdint.h>

struct acpi_rsdp_descriptor rsdp;

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

void acpi_print_table_header(void *addr) {
	if (memcmp(((struct acpi_rsdp_descriptor *) addr)->signature,
				"RSD PTR ", 8) == 0) {
		pr_log("RSDP at 0x%8x (v%d %.6s)\n",
				addr,
				((struct acpi_rsdp_descriptor *) addr)->revision,
				//== 0 ? 1 : 2,
				((struct acpi_rsdp_descriptor *) addr)->oem_id);
	} else {
		struct acpi_sdt_header *header = (struct acpi_sdt_header *) addr;

		pr_log("%.4s at 0x%8x (v%d %.6s %.8s)\n",
				header->signature,
				addr,
				header->revision,// == 0 ? 1 : 2,
				header->oem_id,
				header->oem_table_id);
	}
}

/**
 * @brief Detect the Root System Description Pointer (RSDP) in memory.
 *
 * This function searches for the RSDP signature in the memory
 * range 0x00080000 to 0x00081024 and 0x000E0000 to 0x000FFFFF.
 *
 * @return The address of the RSDP if found, NULL otherwise.
 */
int acpi_find_rsdp() {
	struct acpi_rsdp_descriptor *desc;
	// search first 1KB of the EBDA
	uint32_t size = 0x400;
	char *start = (char *) vmm_map_page_phys(0x80000,
			PAGE_PDE_PRESENT | PAGE_PDE_WRITABLE,
			PAGE_PTE_PRESENT | PAGE_PTE_WRITABLE, size);

	if (!start)
		return 1;

	char *end = start + size;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			if (acpi_validate_rsdp((struct acpi_rsdp_descriptor *) start) == 0) {
				goto found;
			}
		}
		start += 16;
	}

	vmm_unmap_page_phys((uint32_t)start, size);

	// search from 0xe0000 to 0xFFFFF
	size = 0x1FFFF;
	start = (char *) vmm_map_page_phys(0xE0000,
			PAGE_PDE_PRESENT | PAGE_PDE_WRITABLE,
			PAGE_PTE_PRESENT | PAGE_PTE_WRITABLE, size);

	if (!start)
		return 1;

	end = start + size;

	while (start < end) {
		if (memcmp(start, "RSD PTR ", 8) == 0) {
			if (acpi_validate_rsdp((struct acpi_rsdp_descriptor *) start) == 0) {
				goto found;
			}
		}
		start += 16;
	}

	vmm_unmap_page_phys((uint32_t)start, size);

	return 1;

found:
	desc = (struct acpi_rsdp_descriptor *) start;
	if (acpi_validate_rsdp(desc) != 0) {
		printk("found RSDP is not valid!\n");
		vmm_unmap_page_phys((uint32_t)start, size);
		return 1;
	}

	// save rsdp
	memcpy(rsdp.signature, desc->signature, 8);
	rsdp.checksum = desc->checksum;
	memcpy(rsdp.oem_id, desc->oem_id, 6);
	rsdp.revision = desc->revision;
	rsdp.rsdt_phy_address = desc->rsdt_phy_address;

	vmm_unmap_page_phys((uint32_t)start, size);
	return 0;
}

uint8_t acpi_parse_root_table() {
	struct acpi_rsdt *rsdt;
	struct acpi_sdt_header *table;
	uint32_t nr_entries = 0;
	uint32_t *rsdt_va;

	//if (rsdp.revision > 0) {
	//	pr_log("ACPI version 2, using XSDT table not implemented!\n");
	//	return 1;
	//}

	if (rsdp.rsdt_phy_address == 0) {
		pr_log("Invalid RSDT physical address!\n");
		return 1;
	}

	uint32_t rsdt_pa = rsdp.rsdt_phy_address;
	uint32_t rsdt_pa_offset = PAGE_OFFSET(rsdt_pa);
	rsdt_va = vmm_map_page_phys(rsdt_pa, PAGE_PDE_PRESENT, PAGE_PTE_PRESENT, 1);

	if (!rsdt_va) {
		pr_log("Could not map RSDT\n");
		return 1;
	}

	// rsdt phy address might not be aligned
	rsdt = (struct acpi_rsdt *) ((uint32_t)rsdt_va + rsdt_pa_offset);

	// validate table length
	if (rsdt->header.length < (sizeof(struct acpi_sdt_header) + sizeof(uint32_t))) {
		printk("length: %d\n", rsdt->header.length);
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
		uint32_t table_pa = rsdt->pointer_to_other_sdt[i];
		uint32_t table_pa_offset = PAGE_OFFSET(table_pa);

		uint32_t *table_va = vmm_map_page_phys(table_pa, PAGE_PDE_PRESENT, PAGE_PTE_PRESENT, 1);

		if (!table_va) {
			pr_log("Could not map found acpi table\n");
			return 1;
		}

		// table's phy address might not be aligned
		table = (struct acpi_sdt_header *) ((uint32_t)table_va + table_pa_offset);

		if (table && acpi_compute_checksum(table) == 0) {
			acpi_print_table_header(table);

			// add table info in global list
		} else {
			pr_log("%.4s: wrong checksum!\n", table->signature);
		}

		vmm_unmap_page_phys((uint32_t)table_va, 1);
	}

	return 0;
}

/**
 * @brief Discover ACPI Tables
 * @return 0 if successful, 1 otherwise
 */
uint8_t acpi_init() {
	pr_log("Initializing ACPI\n");
	if (acpi_find_rsdp() != 0) {
		return 1;
	}

	acpi_print_table_header(&rsdp);

	if (acpi_parse_root_table() != 0) {
		return 1;
	}

	return 0;
}
