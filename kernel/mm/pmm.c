/* Physical memory manager */
#define pr_log_fmt(msg)	"PMM: " msg
#include <kernel/multiboot.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <mm/pmm.h>

#include <stddef.h>

static uint8_t bitmap[BITMAP_SIZE];
static uint32_t max_blocks;
static uint32_t used_blocks;

// defined in the linker script
extern char _kernel_end_phys[];
extern char _kernel_start_phys[];

int ceil(int a, int b) {
	return (a + b - 1) / b;
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
	// first get the 8-bit chunk of indices in the bitmap where the given index is
	uint32_t indices_chunk = block_index / 8;

	// get the offset of the index in the chunk
	uint32_t index_offset = block_index % 8;

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
	uint32_t indices_chunk = block_index / 8;

	// get the offset of the index in the chunk
	uint32_t index_offset = block_index % 8;

	// set that bit
	bitmap[indices_chunk] &= ~(1 << index_offset);
}

/**
 * @brief Get bit at block_index in the bitmap
 *
 * This function returns the state of the bit describing the block at
 * block_index index in the bitmap.
 *
 * @param block_index 	The index of the block to be freed. The index is
 * 						obtained by dividing the address to the BLOCK_SIZE
 * @return 0 if the bit is unset, 1 otherwise
 */
uint8_t __get_bit(uint32_t block_index) {
	// first get the 32-bit chunk of indices in the bitmap where the given index
	// is
	uint32_t indices_chunk = block_index / 8;

	// get the offset of the index in the chunk
	uint32_t index_offset = block_index % 8;

	return (bitmap[indices_chunk] & (1 << index_offset)) != 0;
}

/**
 * @brief Mark region described by base address and size as free
 *
 * This function unsets the corresponding bits to the given region in
 * the bitmap.
 *
 * !! Make sure that the given size is multiple of BLOCK_SIZE. The
 * function only marks entire blocks as free, so even if a portion
 * of a block should be freed, the entire block will be freed.
 *
 * @param base_addr The abse address of the region
 * @param size		The size of the region
 */
void __mark_region_free(uint32_t base_addr, uint32_t size) {
	uint32_t block_index = base_addr / BLOCK_SIZE;
	uint32_t num_blocks = size / BLOCK_SIZE;

	if (size % BLOCK_SIZE) {
		num_blocks++;
	}

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
 * !! Make sure that the given size is multiple of BLOCK_SIZE. The
 * function only marks entire blocks as reserved, so even if a portion
 * of a block should be reserved, the entire block will be marked.
 *
 * @param base_addr The base address of the region
 * @param size		The size of the region
 */
void __mark_region_reserved(uint32_t base_addr, uint32_t size) {
	uint32_t block_index = base_addr / BLOCK_SIZE;
	uint32_t num_blocks = size / BLOCK_SIZE;

	if (size % BLOCK_SIZE) {
		num_blocks++;
	}

	for (; num_blocks > 0; num_blocks--) {
		__set_block(block_index);
		block_index++;
		used_blocks++;
	}
}

/**
 * @brief Mark regions from the memory map as free or reserved in the bitmap
 *
 * This function goes through the memory map created by E820 two times, the
 * first time marking the free blocks and the second time the reserved ones.
 * This ensures that overlapping parts in the map will be reserved.
 *
 * @param addr		Addr where the memory map starts
 * @param length	Total size of buffer
 */
void pmm_mark_e820_regions(uint32_t addr, uint32_t length) {
	struct multiboot_mmap_entry *mmap_entry;

	for (mmap_entry = (struct multiboot_mmap_entry *) addr;
			(unsigned long) mmap_entry < addr + length;
			mmap_entry = (struct multiboot_mmap_entry *) ((unsigned long) mmap_entry +
				mmap_entry->size + sizeof(mmap_entry->size))) {
		if (mmap_entry->type == 1) {
			__mark_region_free(mmap_entry->base_addr,
							   mmap_entry->length);
		}
	}

	for (mmap_entry = (struct multiboot_mmap_entry *) addr;
			(unsigned long) mmap_entry < addr + length;
			mmap_entry = (struct multiboot_mmap_entry *) ((unsigned long) mmap_entry +
				mmap_entry->size + sizeof(mmap_entry->size))) {
		if (mmap_entry->type != 1) {
			__mark_region_reserved(mmap_entry->base_addr,
							   mmap_entry->length);
		}
	}
}

/**
 * @brief Perform some tests for the memmory manager
 *
 * This function performs some basic test to see if the physical
 * memory manager works as expected.
 */
uint8_t pmm_self_test() {
	uint32_t test_used_blocks = used_blocks;
	uint32_t test_free_blocks = max_blocks - used_blocks;

	uint32_t *a = (uint32_t *) pmm_allocate_blocks(1);

	if ((test_free_blocks == 0 && a != NULL) ||
		(test_free_blocks > 0 && a == NULL) ||
		(a != NULL && test_free_blocks - (max_blocks - used_blocks) != 1) ||
		(a != NULL && used_blocks - test_used_blocks != 1)) {
		return 1;
	} else {
		test_free_blocks--;
		test_used_blocks++;
	}

	uint32_t *b = (uint32_t *) pmm_allocate_blocks(2);

	if ((test_free_blocks < 2 && b != NULL) ||
		(test_free_blocks > 2 && b == NULL) ||
		(b != NULL && test_free_blocks - (max_blocks - used_blocks) != 2) ||
		(b != NULL && used_blocks - test_used_blocks != 2)) {
		return 1;
	} else {
		test_free_blocks -= 2;
		test_used_blocks += 2;
	}

	pmm_free_blocks(a, 1);

	if (((max_blocks - used_blocks) - test_free_blocks != 1) ||
		(test_used_blocks - used_blocks != 1)) {
		return 1;
	} else {
		test_free_blocks += 1;
		test_used_blocks -= 1;
	}

	pmm_free_blocks(b, 2);

	if (((max_blocks - used_blocks) - test_free_blocks != 2) ||
		(test_used_blocks - used_blocks != 2)) {
		return 1;
	} else {
		test_free_blocks += 2;
		test_used_blocks -= 2;
	}

	return 0;
}

/**
 * @brief Initialize the Physical Memory Manager
 *
 * This function first calculates the entire size of the discovered RAM
 * by the function E820, by substracting the smallest discovered address
 * from the biggest. Then, the total size of the bitmap is calculated and
 * the bitmap is placed in memory. At the beginning, all regions are set
 * as reserved, then marked as free and then reserved.
 *
 * @param addr		Addr where the memory map starts
 * @param length	Total size of buffer
 * @return 0 if self tests passed successfully, 1 otherwise
 */
uint8_t pmm_init(uint32_t addr, uint32_t length) {
	max_blocks = MAX_RAM_SIZE / BLOCK_SIZE;
	used_blocks = max_blocks;

	// initialize all regions as used_blocks
	memset(bitmap, 0xFF, BITMAP_SIZE);

	// mark regions in the memory map
	pmm_mark_e820_regions(addr, length);

	// reserve kernel region
	__mark_region_reserved((uint32_t) _kernel_start_phys,
			(uint32_t) (_kernel_end_phys - _kernel_start_phys));

	// reserve 0x0000000
	__mark_region_reserved(0x00000000, BLOCK_SIZE);

	// perform some tests to see that everything works as expected
	return pmm_self_test();
}

/**
 * @brief Return first fit block
 *
 * This function goes through the bitmap and returns the first found block
 * which has enough free blocks after it to fulfill the requested requirement
 *
 * @param req_num_blocks Required number of blocks
 *
 * @return 	starting block of the found region. The function returns 0 if no
 * such region has been found (0 is safe to return, as block 0 is always
 * reserved)
 */
uint32_t __find_first_fit(uint32_t req_num_blocks) {
	uint32_t current_number_of_free_blocks = 0;
	uint32_t starting_block = 0;

	for (size_t i = 0; i < max_blocks; i++) {
		if (__get_bit(i) == 0) {
			current_number_of_free_blocks++;

			if (current_number_of_free_blocks >= req_num_blocks) {
				return starting_block;
			}
		} else {
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
 * @param num_blocks Requested number of blocks
 * @return Starting physical address for the requested region
 */
void *pmm_allocate_blocks(uint32_t num_blocks) {
	if (num_blocks == 0) {
		return NULL;
	}

	if (max_blocks - used_blocks < num_blocks) {
		return NULL;
	}

	uint32_t first_fit_block = __find_first_fit(num_blocks);

	if (first_fit_block == 0) {
		return NULL;
	}

	uint32_t first_fit_block_copy = first_fit_block;

	for (; num_blocks > 0; num_blocks--) {
		__set_block(first_fit_block);
		first_fit_block++;
		used_blocks++;
	}

	return (void *) (first_fit_block_copy * BLOCK_SIZE);
}

/**
 * @brief Free "size" blocks starting at the given address
 *
 * @param address 		Starting address
 * @param num_blocks	Number of blocks to free
 */
void pmm_free_blocks(void *address, uint32_t num_blocks) {
	uint32_t block_index = (uint32_t) address / BLOCK_SIZE;

	for (; num_blocks > 0; num_blocks--) {
		__unset_block(block_index);
		block_index++;
		used_blocks--;
	}
}

/**
 * @brief Print information about the physical memory
 */
void print_phymem_info() {
	printk("total number of blocks: %d\n", max_blocks);
	printk("used blocks: %d\n", used_blocks);
	printk("free blocks: %d\n", max_blocks - used_blocks);
	printk("block size: %dB\n", BLOCK_SIZE);
}
