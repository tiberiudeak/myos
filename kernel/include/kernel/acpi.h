#ifndef KERNEL_ACPI_H
#define KERNEL_ACPI_H 1

#include <stdint.h>

typedef struct {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;
} __attribute__((packed)) RSDP_descriptor_t;

/**
 * ACPI System Descriptor Table Header which is common to
 * all the SDTs.
*/
typedef struct ACPISDT_header {
	char Signature[4];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
} ACPISDT_header;

typedef struct {
	struct ACPISDT_header header;
	uint32_t pointer_to_other_SDT[];
} RSDT;

typedef struct GenericAddressStructure
{
  uint8_t AddressSpace;
  uint8_t BitWidth;
  uint8_t BitOffset;
  uint8_t AccessSize;
  uint64_t Address;
} GenericAddressStructure;

typedef struct {
	struct ACPISDT_header header;
	uint32_t FirmwareCtrl;
	uint32_t Dsdt;
	uint8_t Reserved;
	uint8_t ReferredPowerManagementProfile;
	uint16_t SCI_Interrupt;
	uint32_t SMI_CommandPort;
	uint8_t ACPIEnable;
	uint8_t  AcpiDisable;
  uint8_t  S4BIOS_REQ;
  uint8_t  PSTATE_Control;
  uint32_t PM1aEventBlock;
  uint32_t PM1bEventBlock;
  uint32_t PM1aControlBlock;
  uint32_t PM1bControlBlock;
  uint32_t PM2ControlBlock;
  uint32_t PMTimerBlock;
  uint32_t GPE0Block;
  uint32_t GPE1Block;
  uint8_t  PM1EventLength;
  uint8_t  PM1ControlLength;
  uint8_t  PM2ControlLength;
  uint8_t  PMTimerLength;
  uint8_t  GPE0Length;
  uint8_t  GPE1Length;
  uint8_t  GPE1Base;
  uint8_t  CStateControl;
  uint16_t WorstC2Latency;
  uint16_t WorstC3Latency;
  uint16_t FlushSize;
  uint16_t FlushStride;
  uint8_t  DutyOffset;
  uint8_t  DutyWidth;
  uint8_t  DayAlarm;
  uint8_t  MonthAlarm;
  uint8_t  Century;

  uint16_t BootArchitectureFlags;

  uint8_t  Reserved2;
  uint32_t Flags;

  GenericAddressStructure ResetReg;

  uint8_t  ResetValue;
  uint8_t  Reserved3[3];

  uint64_t                X_FirmwareControl;
  uint64_t                X_Dsdt;

  GenericAddressStructure X_PM1aEventBlock;
  GenericAddressStructure X_PM1bEventBlock;
  GenericAddressStructure X_PM1aControlBlock;
  GenericAddressStructure X_PM1bControlBlock;
  GenericAddressStructure X_PM2ControlBlock;
  GenericAddressStructure X_PMTimerBlock;
  GenericAddressStructure X_GPE0Block;
  GenericAddressStructure X_GPE1Block;
} FADT;

void ACPI_init(void);
void *RSDP_detect(void);
int RSDP_validate(RSDP_descriptor_t *rsdp);
void *find_FACP(void *);

#endif /* KERNEL_ACPI_H */
