#include <kernel/string.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

struct kblock_meta *metadata_blk_header;

/**
 * @brief Initialize the metadata blk header struct
 *
 * @param size	The first requested size from kmalloc
 * @return 0 if successful, 1 otherwise
 */
uint8_t kmalloc_init(size_t size) {
	// get necessary number of pages
	uint32_t req_pages = size / PAGE_SIZE;

	if (size % PAGE_SIZE > 0) {
		req_pages++;
	}

	if (req_pages * PAGE_SIZE - METADATA_BLK_SIZE < size) {
		req_pages++;
	}

	void *addr = allocate_pages(req_pages);

	if (!addr) {
		return 1;
	}

	// create metadata
	metadata_blk_header = (struct kblock_meta *) addr;

	metadata_blk_header->size = (req_pages * PAGE_SIZE) - METADATA_BLK_SIZE;
	metadata_blk_header->status = STATUS_FREE;
	metadata_blk_header->id = 0;
	metadata_blk_header->next = NULL;
	metadata_blk_header->prev = NULL;

	return 0;
}

/**
 * @brief Print list
 */
void kmalloc_print_list(void) {
	struct kblock_meta *current = metadata_blk_header;

	while (current != NULL) {
		printk("node %d size: %d, status %d\n", current->id,
				current->size, current->status);
		current = (struct kblock_meta *) current->next;
	}
}

/**
 * @brief Find best block that fits the requested size
 *
 * @param size The requested size
 * @return The block address if one is found, NULL otherwise
 */
void *kmalloc_find_best_fit(uint32_t size) {
	struct kblock_meta *current = metadata_blk_header;
	struct kblock_meta *best_fit = NULL;

	uint32_t min = 0xFFFFFFFF;

	while (current != NULL) {
		if (current->status == STATUS_FREE && current->size >= size) {
			if (current->size < min) {
				min = current->size;
				best_fit = current;
			}
		}

		current = (struct kblock_meta *) current->next;
	}

	return best_fit;
}

/**
 * @brief Split the given block in two according to the given size
 *
 * @param block Pointer to the block that is to be split
 * @param size  The size that the allocated block has to have
 *
 * @return The address of the block that is allocated (same as the parameter)
 */
void *kmalloc_split_block(struct kblock_meta *block, uint32_t size) {
	struct kblock_meta *new_block =
		(void *) block + METADATA_BLK_SIZE + ALIGN(size, ALIGNMENT);

	new_block->size = block->size - ALIGN(size, ALIGNMENT) - METADATA_BLK_SIZE;
	new_block->status = STATUS_FREE;
	new_block->next = block->next;
	new_block->prev = (struct kblock_meta *) block;
	new_block->id = block->id;

	block->size = block->size - new_block->size - METADATA_BLK_SIZE;
	block->next = (struct kblock_meta *) new_block;

	return block;
}

/**
 * @brief Request more memory from the vmm
 *
 * @param size The requested size
 * @return Pointer to the node in the list that accommodated the requested size
 */
void *kmalloc_expand_memory(uint32_t size) {
	struct kblock_meta *last = metadata_blk_header;

	while (last != NULL && last->next != NULL) {
		last = (struct kblock_meta *) last->next;
	}

	// get necessary number of pages
	uint32_t req_pages = size / PAGE_SIZE;

	if (size % PAGE_SIZE > 0) {
		req_pages++;
	}

	if (req_pages * PAGE_SIZE - METADATA_BLK_SIZE < size) {
		req_pages++;
	}

	void *addr = allocate_pages(req_pages);

	if (!addr) {
		return NULL;
	}

	struct kblock_meta *new_block = (struct kblock_meta *) addr;
	new_block->size = (req_pages * PAGE_SIZE) - METADATA_BLK_SIZE;
	new_block->status = STATUS_FREE;
	// get different id, as it could happen that the memory between
	// these kblock_metas is not contiguous
	new_block->id = last->id == 1 ? 0 : 1;
	new_block->next = NULL;
	new_block->prev = (struct kblock_meta *) last;

	last->next = (struct kblock_meta *) new_block;

	if (new_block->size - ALIGN(size, ALIGNMENT) >=
		METADATA_BLK_SIZE + ALIGN(1, ALIGNMENT)) {
		return kmalloc_split_block(new_block, size);
	}

	return new_block;
}

/**
 * @brief Allocate dynamic memory (called by the kernel)
 *
 * This function initializes the memory metadata structure if necessary and
 * searches for the best fit for the requested size. If a best fit is found,
 * then the block is split if possible. If one is not found, then the memory is
 * expanded.
 *
 * @param size The requested size in bytes
 * @return Starting virtual address
 */
void *kmalloc(size_t size) {
	if (size == 0) {
		return NULL;
	}

	int ret;

	// initialize metadata if necessary
	if (metadata_blk_header == NULL) {
		ret = kmalloc_init(size);
		if (ret) {
			return NULL;
		}
	}

	// find best fit
	struct kblock_meta *best_fit =
		(struct kblock_meta *) kmalloc_find_best_fit(size);

	if (best_fit != NULL) {
		// split block if there is place for at least 8 bytes + sizeof metadata block
		if (best_fit->size - ALIGN(size, ALIGNMENT) >=
			METADATA_BLK_SIZE + ALIGN(1, ALIGNMENT)) {
			struct kblock_meta *split_block =
				kmalloc_split_block(best_fit, size);

			split_block->status = STATUS_ALLOC;

			return (void *) split_block + METADATA_BLK_SIZE;
		}

		best_fit->status = STATUS_ALLOC;
		return (void *) best_fit + METADATA_BLK_SIZE;
	}

	// expand memory
	struct kblock_meta *block = kmalloc_expand_memory(size);

	if (block == NULL) {
		return NULL;
	}

	block->status = STATUS_ALLOC;

	return (void *) block + METADATA_BLK_SIZE;
}

/**
 * @brief Free dynamic memory (called by the kernel)
 *
 * This function goes through the list and searches for the given virtual
 * address. If it is found, the status of the block is changed to STATUS_FREE
 * and the block is coalesced with its predecessor and successor (if possible)
 *
 * @param ptr   Virtual address
 */
void kfree(void *ptr) {
	if (ptr == NULL) {
		return;
	}

	struct kblock_meta *current = metadata_blk_header;

	while (current != NULL) {
		if ((void *) current + METADATA_BLK_SIZE == ptr) {
			current->status = STATUS_FREE;

			memset((void *) current + METADATA_BLK_SIZE, 1, current->size);

			// coalesce if possible
			if (current->prev != NULL && current->prev->status == STATUS_FREE &&
				current->next != NULL && current->next->status == STATUS_FREE &&
				current->prev->id == current->id && current->id == current->next->id) {
				current->prev->size +=
					current->size + current->next->size + 2 * METADATA_BLK_SIZE;
				current->prev->next = current->next->next;

				if (current->next->next != NULL) {
					current->next->next->prev = current->prev;
				}

				return;
			}

			if (current->prev != NULL && current->prev->status == STATUS_FREE &&
				current->prev->id == current->id) {
				current->prev->size += current->size + METADATA_BLK_SIZE;
				current->prev->next = current->next;

				if (current->next != NULL) {
					current->next->prev = current->prev;
				}

				return;
			}

			if (current->next != NULL && current->next->status == STATUS_FREE &&
				current->next->id == current->id) {
				current->size += current->next->size + METADATA_BLK_SIZE;

				if (current->next->next != NULL) {
					current->next->next->prev = current;
				}
				current->next = current->next->next;

				return;
			}

			return;
		}

		current = (struct kblock_meta *) current->next;

		// possible TODO: if size is at least PAGE_SIZE, the page
		// could also be freed from the vmm with free_pages
	}
}
