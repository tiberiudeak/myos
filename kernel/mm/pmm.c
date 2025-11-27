/* Physical memory manager */
#define pr_log_fmt(msg)	"pmm: " msg
#include <kernel/multiboot.h>
#include <kernel/string.h>
#include <kernel/utils.h>
#include <kernel/tty.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>

#include <stddef.h>

uint8_t pmm_bitmap[BITMAP_SIZE];
static uint32_t max_blocks;
static uint32_t used_blocks;

// defined in the linker script
extern char _kernel_end_phys[];
extern char _kernel_start_phys[];
extern char _kernel_end[];

int ceil(int a, int b) {
	return (a + b - 1) / b;
}

/**
 * @brief Set bit corresponding to the given index
 * (corresponding to a block/page)
 *
 * @param bitmap		Bitmap - uint8_t array
 * @param b_size		Size of given bitmap
 * @param index		 	The index of the block/page to be reserved
 * @return 0 if successful, 0 otherwise
 */
int set_bit_in_bitmap(uint8_t *bitmap, uint32_t b_size, uint32_t index) {
	// first get the 8-bit chunk of indices in the bitmap where the given index is
	uint32_t indices_chunk = index / 8;

	// get the offset of the index in the chunk
	uint32_t index_offset = index % 8;

	if (indices_chunk >= b_size) {
		return 1;
	}

	// set that bit
	bitmap[indices_chunk] |= (1 << index_offset);
	return 0;
}

/**
 * @brief Clear bit corresponding to the given index
 * (corresponding to a block/page)
 *
 * @param bitmap		Bitmap - uint8_t array
 * @param b_size		Size of given bitmap
 * @param index		 	The index of the block/page to be reserved
 * @return 0 if successful, 0 otherwise
 */
int unset_bit_in_bitmap(uint8_t *bitmap, uint32_t b_size, uint32_t index) {
	// first get the 32-bit chunk of indices in the bitmap where the given index is
	uint32_t indices_chunk = index / 8;

	// get the offset of the index in the chunk
	uint32_t index_offset = index % 8;

	if (indices_chunk >= b_size) {
		return 1;
	}

	// set that bit
	bitmap[indices_chunk] &= ~(1 << index_offset);
	return 0;
}

/**
 * @brief Get bit corresponding to the given index
 * (corresponding to a block/page)
 *
 * @param bitmap		Bitmap - uint8_t array
 * @param b_size		Size of given bitmap
 * @param index		 	The index of the block/page to be reserved
 * @return bit value, -1 otherwise
 */
int get_bit_from_bitmap(uint8_t *bitmap, uint32_t b_size, uint32_t index) {
	// first get the 32-bit chunk of indices in the pmm_bitmap where the given index
	// is
	uint32_t indices_chunk = index / 8;

	// get the offset of the index in the chunk
	uint32_t index_offset = index % 8;

	if (indices_chunk >= b_size) {
		return -1;
	}

	return (bitmap[indices_chunk] & (1 << index_offset)) != 0;
}

/**
 * @brief Mark region described by base address and size as free
 *		in the given bitmap
 *
 * The function marks only regions which are multiple of 4KiB. So
 * even if the size is not a multiple of 4KiB, the region will
 * be rounded up
 *
 * @param bitmap		Bitmap - uint8_t array
 * @param b_size		Size of given bitmap
 * @param base_addr		Base address of the region
 * @param size			Size of the region
 * @return 0 if successful, 1 otherwise
 */
int mark_region_free(uint8_t *bitmap, uint32_t b_size, uint32_t base_addr,
		uint32_t size) {
	uint32_t block_index = base_addr / BLOCK_SIZE;
	uint32_t num_blocks = size / BLOCK_SIZE;

	if (size % BLOCK_SIZE) {
		num_blocks++;
	}

	for (; num_blocks > 0; num_blocks--) {
		if (unset_bit_in_bitmap(bitmap, b_size, block_index)) {
			return 1;
		}
		block_index++;
		used_blocks--;
	}

	return 0;
}

/**
 * @brief Mark region described by base address and size as reserved
 *		in the given bitmap
 *
 * The function marks only regions which are multiple of 4KiB. So
 * even if the size is not a multiple of 4KiB, the region will
 * be rounded up
 *
 * @param bitmap		Bitmap - uint8_t array
 * @param b_size		Size of given bitmap
 * @param base_addr		Base address of the region
 * @param size			Size of the region
 * @param offset		Index offset - if the starting address doesn't
 * start at 0 (which will correspond to index 0), provide the offset
 * which should be substracted to get the address corresponding to index 0
 * offset = address / 4KiB
 * e.g. starting address = 0x1000 -> offset = 1
 * @return 0 if successful, 1 otherwise
 */
int mark_region_reserved(uint8_t *bitmap, uint32_t b_size, uint32_t base_addr,
		uint32_t size, uint32_t offset) {
	uint32_t block_index = base_addr / BLOCK_SIZE;
	uint32_t num_blocks = size / BLOCK_SIZE;

	if (size % BLOCK_SIZE) {
		num_blocks++;
	}

	block_index -= offset;

	for (; num_blocks > 0; num_blocks--) {
		if (set_bit_in_bitmap(bitmap, b_size, block_index)) {
			return 1;
		}
		block_index++;
		used_blocks++;
	}

	return 0;
}

/**
 * @brief Mark regions from the memory map as free or reserved in the pmm_bitmap
 *
 * This function goes through the memory map created by E820 two times, the
 * first time marking the free blocks and the second time the reserved ones.
 * This ensures that overlapping parts in the map will be reserved.
 *
 * @param addr		Addr where the memory map starts
 * @param length	Total size of buffer
 * @return 0 if successful, 1 otherwise
 */
int pmm_mark_e820_regions(uint32_t addr, uint32_t length) {
	struct multiboot_mmap_entry *mmap_entry;

	for (mmap_entry = (struct multiboot_mmap_entry *) addr;
			(unsigned long) mmap_entry < addr + length;
			mmap_entry = (struct multiboot_mmap_entry *) ((unsigned long) mmap_entry +
				mmap_entry->size + sizeof(mmap_entry->size))) {
		if (mmap_entry->type == 1) {
			if (mark_region_free(pmm_bitmap, BITMAP_SIZE, mmap_entry->base_addr,
							   mmap_entry->length)) {
				return 1;
			}
		}
	}

	for (mmap_entry = (struct multiboot_mmap_entry *) addr;
			(unsigned long) mmap_entry < addr + length;
			mmap_entry = (struct multiboot_mmap_entry *) ((unsigned long) mmap_entry +
				mmap_entry->size + sizeof(mmap_entry->size))) {
		if (mmap_entry->type != 1) {
			if (mark_region_reserved(pmm_bitmap, BITMAP_SIZE, mmap_entry->base_addr,
							   mmap_entry->length, 0)) {
				return 1;
			}
		}
	}

	return 0;
}

/**
 * @brief Perform some basic tests to see if the physical
 * memory manager works as expected
 *
 * @return 0 if successful, 1 otherwise
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
 * @param addr		Addr where the memory map starts
 * @param length	Total size of buffer
 * @return 0 if successful, 1 otherwise
 */
uint8_t pmm_init(uint32_t addr, uint32_t length) {
	max_blocks = MAX_RAM_SIZE / BLOCK_SIZE;
	used_blocks = max_blocks;

	// initialize all regions as used_blocks
	memset(pmm_bitmap, 0xFF, BITMAP_SIZE);

	// mark regions in the memory map
	if (pmm_mark_e820_regions(addr, length)) {
		return 1;
	}

	// reserve kernel region
	if (mark_region_reserved(pmm_bitmap, BITMAP_SIZE, (uint32_t) _kernel_start_phys,
			(uint32_t) (_kernel_end_phys - _kernel_start_phys), 0)) {
		return 1;
	}

	// reserve first MB as well
	if (mark_region_reserved(pmm_bitmap, BITMAP_SIZE, 0x00000000, 0x100000, 0))
		return 1;

	// perform some tests to see that everything works as expected
	return pmm_self_test();
}

/**
 * @brief Return first fit 4KiB region in the given bitmap
 *
 * @param bitmap		Bitmap - uint8_t array
 * @param b_size		Size of given bitmap
 * @param n				Number of 4KiB regions to iterate through
 * @return index of found region (if any), 0 otherwise
 * (0 should no be an available region, so make sure to reserve it beforehand)
 */
uint32_t find_first_fit(uint8_t *bitmap, uint32_t b_size, uint32_t n, uint32_t req_nr) {
	uint32_t current_number_of_free_regs = 0;
	uint32_t starting_reg = 0;

	for (size_t i = 0; i < n; i++) {
		if (get_bit_from_bitmap(bitmap, b_size, i) == 0) {
			current_number_of_free_regs++;

			if (current_number_of_free_regs >= req_nr) {
				return starting_reg;
			}
		} else {
			   current_number_of_free_regs = 0;
			   starting_reg = i + 1;
		}
	}

	// no memory available
	return 0;
}

/**
 * @brief Allocate requested nr of blocks
 *
 * Reqion of physical memory will be contiguous
 *
 * @param nr_blocks	Requested number of blocks
 * @return physical address of the block if any are free,
 * NULL otherwise
 */
void *pmm_allocate_blocks(uint32_t nr_blocks) {
	if (max_blocks - used_blocks < nr_blocks) {
		return NULL;
	}

	uint32_t first_fit_block = find_first_fit(pmm_bitmap, BITMAP_SIZE,
			max_blocks, nr_blocks);

	// first block is reserved already
	if (first_fit_block == 0) {
		return NULL;
	}

	uint32_t first_fit_block_copy = first_fit_block;

	for (uint32_t i = 0; i < nr_blocks; i++) {
		set_bit_in_bitmap(pmm_bitmap, BITMAP_SIZE, first_fit_block);
		first_fit_block++;
		used_blocks++;
	}

	return (void *) (first_fit_block_copy * BLOCK_SIZE);
}

/**
 * @brief Free nr_blocks blocks starting at given physical address
 * @param address		Physical address
 * @param nr_blocks		Number of blocks
 */
void pmm_free_blocks(void *address, uint32_t nr_blocks) {
	uint32_t block_index = (uint32_t) address / BLOCK_SIZE;
	for (uint32_t i = 0; i < nr_blocks; i++) {
		unset_bit_in_bitmap(pmm_bitmap, BITMAP_SIZE, block_index);
		block_index++;
		used_blocks--;
	}
}

/**
 * @brief Print information about the physical memory
 */
void print_phymem_info() {
	pr_log("total number of blocks: %d\n", max_blocks);
	pr_log("used blocks: %d\n", used_blocks);
	pr_log("free blocks: %d\n", max_blocks - used_blocks);
}

/**
 * @brief Some basic tests for the page allocator
 *
 * - the function is defined here to have access to the
 *   max_blocks and used_blocks variables from the pmm
 */
void test_page_allocator(void) {
	uint32_t initial_used = used_blocks;
	void *tmp;

	void *p1 = allocate_pages(1);
	tmp = p1;
	ASSERT(p1 != NULL, "Failed to allocate 1 page");
	ASSERT(used_blocks == initial_used + 1, "used_blocks mismatch after 1 page alloc");

	void *p2 = allocate_pages(2);
	ASSERT(p2 != NULL, "Failed to allocate 2 pages");
	ASSERT(used_blocks == initial_used + 3, "used_blocks mismatch after 2 page alloc");

	free_pages(p1, 1);
	ASSERT(used_blocks == initial_used + 2, "used_blocks mismatch after freeing 1 page");
	free_pages(p2, 2);
	ASSERT(used_blocks == initial_used, "used_blocks mismatch after freeing 2 pages");

	p1 = allocate_pages(1);
	ASSERT(p1 != NULL, "Failed to allocate 1 page again");
	ASSERT(p1 == tmp, "Unexpected virtual address");
	free_pages(p1, 1);

	void *blocks[10];
	for (int i = 0; i < 10; i++) {
		blocks[i] = allocate_pages(i + 1);
		ASSERT(blocks[i] != NULL, "Failed multiple allocation");
		ASSERT(used_blocks <= max_blocks, "used_blocks exceeded max_blocks");
	}

	for (int i = 0; i < 10; i++) {
		free_pages(blocks[i], i + 1);
		ASSERT(used_blocks >= initial_used, "used_blocks below initial");
	}
	ASSERT(used_blocks == initial_used, "used_blocks not back to initial");

	uint32_t free_blocks_count = max_blocks - used_blocks;
	ASSERT(free_blocks_count == max_blocks - initial_used, "Free block count mismatch");
}
