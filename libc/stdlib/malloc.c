#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct block_meta *metadata_blk_header = NULL;

struct block_meta *last_block = NULL;

/**
 * @brief Initialize memory managed by malloc
 *
 * This function makes the first request of memory using sbrk. It
 * requests INITIAL_REQ_SIZE bytes, that will be later managed internally.
 */
void malloc_init(void) {
	metadata_blk_header = sbrk(INITIAL_REQ_SIZE);

	if (metadata_blk_header == (void *) -1) {
		printf("sbrk error\n");
		return;
	}

	metadata_blk_header->size = ALIGN(INITIAL_REQ_SIZE) - METADATA_BLK_SIZE;
	metadata_blk_header->status = STATUS_FREE;
	metadata_blk_header->next = NULL;
}

/**
 * @brief Find best fit block of memory for the requested size
 *
 * This function goes through all blocks of memory and searches the one
 * that is free and has the smallest size that cna fit the requested size.
 *
 * @param size The requested size
 *
 * @return Block address that fits the requested size
 */
void *malloc_best_fit(size_t size) {
	struct block_meta *current = metadata_blk_header;
	struct block_meta *best_fit = NULL;
	size_t min = 0xFFFFFFFF;

	while (current != NULL) {
		if (size <= current->size && current->status == STATUS_FREE) {
			if (current->size < min) {
				min = current->size;
				best_fit = current;
			}
		}

		current = current->next;
	}

	return best_fit;
}

#ifdef CONFIG_UVMM_FIRSTFIT
/**
 * @brief Find first fit block of memory for the requested size
 *
 * This function goes through all blocks of memory and returns the first
 * block that is free and has the size that can fit the requested size.
 *
 * @param size The requested size
 *
 * @return Block address that fits the requested size
 */
void *malloc_first_fit(size_t size) {
	struct block_meta *current = metadata_blk_header;

	while (current != NULL) {
		if (size <= current->size && current->status == STATUS_FREE) {
			return current;
		}

		current = current->next;
	}

	return NULL;
}
#endif

#ifdef CONFIG_UVMM_WORSTFIT
/**
 * @brief Find worst fit block of memory for the requested size
 *
 * This function goes through all blocks of memory and searches the one
 * that is free and has the largest size that can fit the requested size.
 *
 * @param size The requested size
 *
 * @return Block address that fits the requested size
 */
void *malloc_worst_fit(size_t size) {
	struct block_meta *current = metadata_blk_header;
	struct block_meta *worst_fit = NULL;
	size_t max = 0;

	while (current != NULL) {
		if (size <= current->size && current->status == STATUS_FREE) {
			if (current->size > max) {
				max = current->size;
				worst_fit = current;
			}
		}

		current = current->next;
	}

	return worst_fit;
}
#endif

#ifdef CONFIG_UVMM_NEXTFIT
/**
 * @brief Find next fit block of memory for the requested size
 *
 * This function goes through all blocks of memory and searches the one
 * that is free and has the smallest size that can fit the requested size.
 * The starting point of the search is the last block that was allocated.
 * If no block is found, the function will start searching from the beginning
 *
 * @param size The requested size
 *
 * @return Block address that fits the requested size
 */
void *malloc_next_fit(size_t size) {
	struct block_meta *current = last_block;

	while (current != NULL) {
		if (size <= current->size && current->status == STATUS_FREE) {
			return current;
		}

		current = current->next;
	}

	current = metadata_blk_header;

	while (current != last_block) {
		if (size <= current->size && current->status == STATUS_FREE) {
			last_block = current;
			return current;
		}

		current = current->next;
	}

	return NULL;
}
#endif

/**
 * @brief Print current memory blocks managed by malloc
 *
 * This function goes through all memory blocks managed by malloc and
 * prints their size and status.
 */
void print_malloc_list(void) {
	struct block_meta *tmp = metadata_blk_header;

	while (tmp != NULL) {
		printf("size: %d, status %d\n", tmp->size, tmp->status);
		tmp = tmp->next;
	}
}

/**
 * @brief Split given block in two - one block that has the given size and
 * another one that will have the remaining size
 *
 * This function splits the given block in two. The first block will have the
 * given size and the second one will have the remaining size (after the given
 * size is substracted from the initial size of the block).
 *
 * @param block The block to be split
 * @param size  The size of the first block
 *
 * @return Address of data in the first block
 */
void *malloc_split_block(struct block_meta *block, size_t size) {
	struct block_meta *new_block =
		(void *) block + METADATA_BLK_SIZE + ALIGN(size);

	new_block->size = block->size - ALIGN(size) - METADATA_BLK_SIZE;
	new_block->status = STATUS_FREE;
	new_block->next = block->next;

	block->size = ALIGN(size);
	block->status = STATUS_ALLOC;
	block->next = new_block;

	return (void *) block + METADATA_BLK_SIZE;
}

/**
 * @brief Request memory using sbrk and add it at to the list
 *
 * This function requests more memory using sbrk. If the last block in the
 * list is free, then that block is expanded (with the needed size). Otherwise,
 * a new block is created at the end of the list.
 *
 * @param size  The size with which malloc was called
 *
 * @return 0 if successfully expanded memory, -1 otherwise
 */
uint8_t expand_last_block(size_t size) {
	struct block_meta *current = metadata_blk_header;

	while (current->next != NULL) {
		current = current->next;
	}

	if (current == NULL) {
		return -1;
	}

	if (current->status == STATUS_FREE) {
		// expand last block
		size_t needed_size = ALIGN(size) - current->size;
		void *addr = sbrk(needed_size);

		if (addr == (void *) -1) {
			printf("sbrk error\n");
			return -1;
		}

		current->size += ALIGN(needed_size);
	} else {
		// add new block with new free memory
		size_t needed_size = ALIGN(size) + METADATA_BLK_SIZE;
		void *addr = sbrk(needed_size);

		if (addr == (void *) -1) {
			printf("sbrk error!\n");
			return -1;
		}

		struct block_meta *new_block = (struct block_meta *) addr;
		new_block->size = ALIGN(size);
		new_block->status = STATUS_FREE;
		new_block->next = NULL;

		current->next = new_block;
	}

	return 0;
}

/**
 * @brief Allocate dynamic memory (called by user processes)
 *
 * This function initializes the memory metadata structure if necessary and
 * searches for a block of memory for the requested size. If a block is found,
 * then the block is split if possible. If no block is found, the function
 * tries to expand its managed memory and try to allocate again.
 *
 * @param size  The requested size in bytes
 *
 * @return Starting virtual address
 */
void *malloc(size_t size) {
	struct block_meta *block = NULL;

	if (size == 0) {
		return NULL;
	}

	// init malloc structure
	if (metadata_blk_header == NULL) {
		malloc_init();
	}

alloc_malloc:
	// find block for the requested size
#ifdef CONFIG_UVMM_BESTFIT
	block = malloc_best_fit(size);
#elif CONFIG_UVMM_FIRSTFIT
	block = malloc_first_fit(size);
#elif CONFIG_UVMM_WORSTFIT
	block = malloc_worst_fit(size);
#elif CONFIG_UVMM_NEXTFIT
	block = malloc_next_fit(size);
#else
	block = malloc_best_fit(size);
#endif

	if (block != NULL) {
		// split block if possible and return address
		// split block if at least 8 more bytes can be allocated
		if (block->size - size >= METADATA_BLK_SIZE + ALIGN(1)) {
			return malloc_split_block(block, size);
		}

		// block cannot be split
		block->status = STATUS_ALLOC;
		return (void *) block + METADATA_BLK_SIZE;
	}

	// no best fit block found, increase program break with sbrk
	int ret = expand_last_block(size);

	// if last block expanded successfully, try to allocate again
	if (ret == 0) {
		goto alloc_malloc;
	}

	return NULL;
}

/**
 * @brief Free dynamic memory (called by user processes)
 *
 * This function goes through the list of block and searches for the given
 * virtual address. If the address is found, the status of the block is changed
 * to free and the block is coalesced with its predecessor and successor
 * if possible.
 *
 * @param ptr   The virtual address
 */
void free(void *ptr) {
	if (ptr == NULL) {
		return;
	}

	struct block_meta *current = metadata_blk_header;
	struct block_meta *prev = NULL;

	while (current != NULL) {
		if ((void *) current + METADATA_BLK_SIZE == ptr) {
			current->status = STATUS_FREE;

			// coalesce blocks if possible
			if (prev != NULL && prev->status == STATUS_FREE &&
				current->next != NULL && current->next->status == STATUS_FREE) {
				prev->size +=
					current->size + current->next->size + 2 * METADATA_BLK_SIZE;
				prev->next = current->next->next;
				return;
			}

			if (prev != NULL && prev->status == STATUS_FREE) {
				prev->size += current->size + METADATA_BLK_SIZE;
				prev->next = current->next;
				return;
			}

			if (current->next != NULL && current->next->status == STATUS_FREE) {
				current->size += current->next->size + METADATA_BLK_SIZE;
				current->next = current->next->next;
				return;
			}

			return;
		}

		prev = current;
		current = (struct block_meta *) current->next;
	}
}
