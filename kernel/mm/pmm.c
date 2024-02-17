/* Physical memory manager */
#include <mm/pmm.h>
#include <kernel/spinlock.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

static uint32_t *bitmap;
static uint32_t bitmap_size;
static uint32_t max_blocks;
static uint32_t used_blocks;

atomic_flag pmm_lock = ATOMIC_FLAG_INIT;

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

/**
 * @brief Set block as reserved
 *
 * This function sets the given block as reserved in the bitmap.
 *
 * @param block_index 	The index of the block to be reserved. The index is
 * 						obtained by dividing the address to the BLOCK_SIZE
 */
void __set_block(uint32_t block_index) {
	// first get the 32-bit chunk of indices in the bitmap where the given index is
	uint32_t indices_chunk = block_index / 32;

	// get the offset of the index in the chunk
	uint32_t index_offset = block_index % 32;

	// set that bit
	bitmap[indices_chunk] |= (1 << index_offset);
}

/**
 * @brief Set block as free
 *
 * This function sets the given block as free in the bitmap.
 *
 * @param block_index 	The index of the block to be freed. The index is
 * 						obtained by dividing the address to the BLOCK_SIZE
 */
void __unset_block(uint32_t block_index) {
	// first get the 32-bit chunk of indices in the bitmap where the given index is
	uint32_t indices_chunk = block_index / 32;

	// get the offset of the index in the chunk
	uint32_t index_offset = block_index % 32;

	// set that bit
	bitmap[indices_chunk] &= ~(1 << index_offset);
}

/**
 * @brief Get bit at block_index in the bitmap
 *
 * This function returns the state of the bit describing the block at block_index
 * index in the bitmap.
 *
 * @param block_index 	The index of the block to be freed. The index is
 * 						obtained by dividing the address to the BLOCK_SIZE
 * @return 0 if the bit is unset, 1 otherwise
 */
uint8_t __get_bit(uint32_t block_index) {
	// first get the 32-bit chunk of indices in the bitmap where the given index is
	uint32_t indices_chunk = block_index / 32;

	// get the offset of the index in the chunk
	uint32_t index_offset = block_index % 32;

	return (bitmap[indices_chunk] & (1 << index_offset)) != 0;
}

/**
 * @brief Mark region described by base address and size as free
 *
 * This function ussets the corresponding bits to the given region in
 * the bitmap.
 *
 * @param base_addr The abse address of the region
 * @param size		The size of the region
 */
void __mark_region_free(uint32_t base_addr, uint32_t size) {
	uint32_t block_index = base_addr / BLOCK_SIZE;
	uint32_t num_blocks = size / BLOCK_SIZE;

	for (; num_blocks > 0; num_blocks--) {
		__unset_block(block_index);
		block_index++;
		used_blocks--;
	}
}

/**
 * @brief Mark region described by base address and size as reserved
 *
 * This function sets the corresponding bits to the given region in
 * the bitmap.
 *
 * @param base_addr The base address of the region
 * @param size		The size of the region
 */
void __mark_region_reserved(uint32_t base_addr, uint32_t size) {
	uint32_t block_index = base_addr / BLOCK_SIZE;
	uint32_t num_blocks = size / BLOCK_SIZE;

	for (; num_blocks > 0; num_blocks--) {
		__set_block(block_index);
		block_index++;
		used_blocks++;
	}
}

/**
 * @brief Mark regions from the memory map as free or reserved in the bitmap
 *
 * This function goesthrough the memory map created by E820 two times, the first
 * time marking the free blocks and the second time the reserved ones. This ensures
 * that overlapping parts in the map will be reserved.
 */
void mark_e820_regions() {
	uint32_t *nr_entries = (uint32_t*) MEM_MAP_NR_ENTRIES_ADDRESS;
	int offset = 0;

	for(size_t i = 0; i < *nr_entries; i++) {
		mem_map_entry_t *mem_map_entry = (mem_map_entry_t*) MEM_MAP_ADDRESS + offset;

		if (mem_map_entry->region_type == 1) {
			__mark_region_free(mem_map_entry->base_addr, mem_map_entry->region_length);
		}

		offset++;
	}

	offset = 0;

	for(size_t i = 0; i < *nr_entries; i++) {
		mem_map_entry_t *mem_map_entry = (mem_map_entry_t*) MEM_MAP_ADDRESS + offset;

		if (mem_map_entry->region_type != 1) {
			__mark_region_reserved(mem_map_entry->base_addr, mem_map_entry->region_length);
		}

		offset++;
	}
}

/**
 *
*/
void pmm_self_test() {

}

/**
 * @brief Initialize the Physical Memory Manager
 *
 * This function first calculates the entire size of the discovered RAM
 * by the function E820, by substracting the smallest discovered address
 * from the biggest. Then, the total size of the bitmap is calculated and
 * the bitmap is placed in memory. At the beginning, all regions are set
 * as reserved, then marked as free and then reserved.
 */
void initialize_memory() {
	// get base address and end address and calculate total size of RAM
	uint64_t base_address;
	uint64_t end_address;

	mem_map_entry_t *mem_map_entry = (mem_map_entry_t*) MEM_MAP_ADDRESS;
	uint32_t *nr_entries = (uint32_t*) MEM_MAP_NR_ENTRIES_ADDRESS;

	base_address = mem_map_entry->base_addr;

	mem_map_entry = (mem_map_entry_t*) MEM_MAP_ADDRESS + (*nr_entries - 1);
	end_address = mem_map_entry->base_addr + mem_map_entry->region_length - 1;

	uint32_t total_ram_size = end_address - base_address;
	printf("total RAM size: %x\n", total_ram_size);

	// calculate bitmap size and place it in memory
	bitmap_size = total_ram_size / BLOCK_SIZE;
	bitmap_size = ceil(bitmap_size, 8);

	printf("bitmap size in bytes: %d\n", bitmap_size);

	bitmap = (uint32_t*) BITMAP_ADDRESS;
	max_blocks = total_ram_size / BLOCK_SIZE;
	used_blocks = max_blocks;

	// initialize all regions as used_blocks
	memset(bitmap, 0xFF, bitmap_size);

	// mark regions in the memory map
	mark_e820_regions();

	// reserve lower part of memory until 0x100000 (kernel, BDA, mem map, etc.)
	__mark_region_reserved(0, 0x100000);

	printf("total number of blocks: %d\n", max_blocks);
	printf("used blocks: %d\n", used_blocks);
	printf("free blocks: %d\n", max_blocks - used_blocks);

	// perform some tests to see that everything works as expected
	pmm_self_test();
}

/**
 * @brief Return first fit block
 *
 * This function goes through the bitmap and returns the first found block
 * that has enough free blocks afterwards to fulfill the requested requirement
 *
 * @param req_num_blocks Required number of blocks
 *
 * @return 	starting block of the found region. The function returns 0 if no such
 * 			region has been found (0 is safe to return, as block 0 is always reserved)
 */
uint32_t __find_first_fit(uint32_t req_num_blocks) {
	uint32_t current_number_of_free_blocks = 0;
	uint32_t starting_block = 0;

	// TODO: optimization: check entire 32-bit or one byte at least at a time
	// instead of going through each bit in the bitmap
	for (size_t i = 0; i < max_blocks; i++) {
		if (__get_bit(i) == 0) {
			current_number_of_free_blocks++;

			if (current_number_of_free_blocks >= req_num_blocks) {
				return starting_block;
			}
		}
		else {
			current_number_of_free_blocks = 0;
			starting_block = i + 1;
		}
	}

	// no memory available
	return 0;
}

/**
 * @brief Allocate num_blocks of physical memory
 *
 * This function allocates the requested number of blocks.
 *
 * @param num_blocks Requested number of blocks
 *
 * @return Starting physical address for the requested region
 */
void *kalloc(uint32_t num_blocks) {
	if (num_blocks == 0) {
		return NULL;
	}

	if (max_blocks - used_blocks < num_blocks) {
		return NULL;
	}

	spinlock_acquire(&pmm_lock);
	uint32_t first_fit_block = __find_first_fit(num_blocks);
	spinlock_release(&pmm_lock);

	if (first_fit_block == 0) {
		return NULL;
	}

	uint32_t first_fit_block_copy = first_fit_block;

	for (; num_blocks > 0; num_blocks--) {
		__set_block(first_fit_block);
		first_fit_block++;
		used_blocks++;
	}

	return (void*)(first_fit_block_copy * BLOCK_SIZE);
}

/**
 * @brief Free "size" blocks starting at the given address
 *
 * This function frees "size" blocks starting at the given address.
 *
 * @param address 		Starting address
 * @param num_blocks	Number of blocks to free
 */
void kfree(void *address, uint32_t num_blocks) {
	uint32_t block_index = (uint32_t)address / BLOCK_SIZE;

	for (; num_blocks > 0; num_blocks--) {
		__unset_block(block_index);
		block_index++;
		used_blocks--;
	}
}

/**
 * @brief Print information about the physical memory
 *
 * Print total number of blocks, used and free blocks and the block size.
 */
void print_phymem_info() {
	printf("total number of blocks: %d\n", max_blocks);
	printf("used blocks: %d\n", used_blocks);
	printf("free blocks: %d\n", max_blocks - used_blocks);
	printf("block size: %dB\n", BLOCK_SIZE);
}
