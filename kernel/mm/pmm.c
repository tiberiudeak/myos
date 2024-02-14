/* Physical memory manager */
#include <mm/pmm.h>
#include <stddef.h>
#include <stdio.h>

/**
 * @brief Print the memory map created by INT 0x15 E820
 *
 * This function prints to the screen the map of the memory created
 * by the bootloader that starts at address 0x1004.
 */
void print_mem_map() {
	uint32_t *nr_entries = (uint32_t*) MEM_MAP_NR_ENTRIES_ADDRESS;
	int offset = 0;

	for (size_t i = 0; i < *nr_entries; i++) {
		mem_map_entry_t *mem_map_entry = (mem_map_entry_t*) MEM_MAP_ADDRESS + offset;

		printf("base: %x ", mem_map_entry->base_addr);
		printf("length: %x ", mem_map_entry->region_length);
		printf("type: %x\n", mem_map_entry->region_type);

		offset += 1;
	}
}
