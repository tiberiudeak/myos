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

		printf("E820: mem [%llx-%llx] %s\n", mem_map_entry->base_addr,
			mem_map_entry->base_addr + mem_map_entry->region_length - 1,
			mem_map_entry->region_type == 1 ? "usable" : "reserved");

		offset += 1;
	}
}
