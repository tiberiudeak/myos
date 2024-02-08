#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H 1

typedef struct {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
} __attribute__((packed)) RSDP_descriptor_t;

void ACPI_init(void);
uint32_t RSDP_detect(void);
int RSDP_validate(uint32_t rsdp);

#endif
