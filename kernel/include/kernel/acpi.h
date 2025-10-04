#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H 1

#include <stdint.h>

struct acpi_table_descriptor {
	uint32_t physical_address;
	struct acpi_sdt_header *header;
};

struct acpi_table_list {
	struct acpi_table_descriptor *tables;
	uint32_t table_count;
};

/* Root System Description Pointer */
struct acpi_rsdp_descriptor {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_phy_address;
} __attribute__((packed));

/**
 * ACPI System Descriptor Table Header common to all SDTs.
 */
struct acpi_sdt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	char creator_id[4];
	uint32_t creator_revision;
} __attribute__((packed));

/* Root System Description Table */
struct acpi_rsdt {
	struct acpi_sdt_header header;
	uint32_t pointer_to_other_sdt[];
} __attribute__((packed));

struct acpi_generic_address {
	uint8_t address_space;
	uint8_t bit_width;
	uint8_t bit_offset;
	uint8_t access_size;
	uint64_t address;
} __attribute__((packed));

/* Fixed ACPI Description  Table */
struct acpi_fadt {
	struct acpi_sdt_header header;
	uint32_t firmware_ctrl;
	uint32_t dsdt;
	uint8_t reserved;
	uint8_t preferred_power_mgmt_profile;
	uint16_t sci_interrupt;
	uint32_t smi_command_port;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t s4_bios_request;
	uint8_t pstate_control;
	uint32_t pm1a_event_block;
	uint32_t pm1b_event_block;
	uint32_t pm1a_control_block;
	uint32_t pm1b_control_block;
	uint32_t pm2_control_block;
	uint32_t pm_timer_block;
	uint32_t gpe0_block;
	uint32_t gpe1_block;
	uint8_t pm1_event_length;
	uint8_t pm1_control_length;
	uint8_t pm2_control_length;
	uint8_t pm_timer_length;
	uint8_t gpe0_length;
	uint8_t gpe1_length;
	uint8_t gpe1_base;
	uint8_t cstate_control;
	uint16_t worst_c2_latency;
	uint16_t worst_c3_latency;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alarm;
	uint8_t month_alarm;
	uint8_t century;

	uint16_t boot_architecture_flags;

	uint8_t reserved2;
	uint32_t flags;

	struct acpi_generic_address reset_reg;

	uint8_t reset_value;
	uint8_t reserved3[3];

	uint64_t x_firmware_control;
	uint64_t x_dsdt;

	struct acpi_generic_address x_pm1a_event_block;
	struct acpi_generic_address x_pm1b_event_block;
	struct acpi_generic_address x_pm1a_control_block;
	struct acpi_generic_address x_pm1b_control_block;
	struct acpi_generic_address x_pm2_control_block;
	struct acpi_generic_address x_pm_timer_block;
	struct acpi_generic_address x_gpe0_block;
	struct acpi_generic_address x_gpe1_block;
} __attribute__((packed));

uint8_t acpi_init(void);

#endif /* KERNEL_ACPI_H */
