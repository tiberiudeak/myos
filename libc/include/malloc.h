#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <stdint.h>

#define STATUS_FREE		  0
#define STATUS_ALLOC	  1

#define ALIGNMENT		  8
#define ALIGN(size)		  (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define METADATA_BLK_SIZE ALIGN(sizeof(block_meta))

#define BLOCK_SIZE		  4096
// initial requested size is one block
#define INITIAL_REQ_SIZE  (BLOCK_SIZE)

struct block_meta {
	size_t size;
	uint8_t status;
	struct block_meta *next;
} block_meta;

#endif /* !MALLOC_H */
