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
void *pmm_allocate_block(void);
void pmm_free_block(void *);
void print_phymem_info(void);

int mark_region_reserved(uint8_t *, uint32_t, uint32_t, uint32_t, uint32_t);
int mark_region_free(uint8_t *, uint32_t, uint32_t, uint32_t);
int set_bit_in_bitmap(uint8_t *, uint32_t, uint32_t);
int unset_bit_in_bitmap(uint8_t *, uint32_t, uint32_t);
int get_bit_from_bitmap(uint8_t *, uint32_t, uint32_t);
uint32_t find_first_fit(uint8_t *, uint32_t, uint32_t);

#endif /* !MM_PMM_H */
