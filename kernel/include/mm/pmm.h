#ifndef MM_PMM_H
#define MM_PMM_H 1

#include <stdint.h>

#define BLOCK_SIZE			4096 // 4K
#define MAX_RAM_SIZE		0x100000000 // 4GB
#define BITMAP_SIZE			((MAX_RAM_SIZE / BLOCK_SIZE) / 8)

struct mem_map_entry {
	uint64_t base_addr;
	uint64_t region_length;
	uint32_t region_type;
	uint32_t acpi;
} __attribute__((packed));

uint8_t pmm_init(uint32_t, uint32_t);
void *allocate_blocks(uint32_t);
void free_blocks(void *, uint32_t);
void print_phymem_info(void);

#endif /* !MM_PMM_H */
