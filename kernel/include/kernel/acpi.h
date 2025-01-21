#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H 1

#include <stdint.h>

struct RSDP_descriptor {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;
} __attribute__((packed));

/**
 * ACPI System Descriptor Table Header which is common to
 * all the SDTs.
 */
struct ACPISDT_header {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
};

struct RSDT {
	struct ACPISDT_header header;
	uint32_t pointer_to_other_SDT[];
};

struct GenericAddressStructure {
	uint8_t AddressSpace;
	uint8_t BitWidth;
	uint8_t BitOffset;
	uint8_t AccessSize;
	uint64_t Address;
};

struct FADT {
	struct ACPISDT_header header;
	uint32_t FirmwareCtrl;
	uint32_t Dsdt;
	uint8_t Reserved;
	uint8_t ReferredPowerManagementProfile;
	uint16_t SCI_Interrupt;
	uint32_t SMI_CommandPort;
	uint8_t ACPIEnable;
	uint8_t AcpiDisable;
	uint8_t S4BIOS_REQ;
	uint8_t PSTATE_Control;
	uint32_t PM1aEventBlock;
	uint32_t PM1bEventBlock;
	uint32_t PM1aControlBlock;
	uint32_t PM1bControlBlock;
	uint32_t PM2ControlBlock;
	uint32_t PMTimerBlock;
	uint32_t GPE0Block;
	uint32_t GPE1Block;
	uint8_t PM1EventLength;
	uint8_t PM1ControlLength;
	uint8_t PM2ControlLength;
	uint8_t PMTimerLength;
	uint8_t GPE0Length;
	uint8_t GPE1Length;
	uint8_t GPE1Base;
	uint8_t CStateControl;
	uint16_t WorstC2Latency;
	uint16_t WorstC3Latency;
	uint16_t FlushSize;
	uint16_t FlushStride;
	uint8_t DutyOffset;
	uint8_t DutyWidth;
	uint8_t DayAlarm;
	uint8_t MonthAlarm;
	uint8_t Century;

	uint16_t BootArchitectureFlags;

	uint8_t Reserved2;
	uint32_t Flags;

	struct GenericAddressStructure ResetReg;

	uint8_t ResetValue;
	uint8_t Reserved3[3];

	uint64_t X_FirmwareControl;
	uint64_t X_Dsdt;

	struct GenericAddressStructure X_PM1aEventBlock;
	struct GenericAddressStructure X_PM1bEventBlock;
	struct GenericAddressStructure X_PM1aControlBlock;
	struct GenericAddressStructure X_PM1bControlBlock;
	struct GenericAddressStructure X_PM2ControlBlock;
	struct GenericAddressStructure X_PMTimerBlock;
	struct GenericAddressStructure X_GPE0Block;
	struct GenericAddressStructure X_GPE1Block;
};

void ACPI_init(void);
void *RSDP_detect(void);
int RSDP_validate(struct RSDP_descriptor *rsdp);
void *find_FACP(void *);

#endif /* KERNEL_ACPI_H */
