#ifndef MM_PMM_H
#define MM_PMM_H 1

#include <stdint.h>

#define MEM_MAP_NR_ENTRIES_ADDRESS		0x1000
#define MEM_MAP_ADDRESS					0x1004

#define BLOCK_SIZE						4096 // 4K
#define BITMAP_ADDRESS					0x60000

typedef struct {
	uint64_t base_addr;
	uint64_t region_length;
	uint32_t region_type;
	uint32_t acpi;
} __attribute__((packed)) mem_map_entry_t;

void print_mem_map(void);
uint8_t initialize_memory(void);
void *allocate_blocks(uint32_t);
void free_blocks(void*, uint32_t);
void print_phymem_info(void);

#endif /* !MM_PMM_H */
