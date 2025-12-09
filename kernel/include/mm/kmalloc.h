#ifndef MM_KMALLOC_H
#define MM_KMALLOC_H 1

#include <stddef.h>
#include <stdint.h>

#define STATUS_FREE			   0
#define STATUS_ALLOC		   1

#define ALIGNMENT			   8
#define ALIGN(size, alignment) (((size) + (alignment - 1)) & ~(alignment - 1))
#define METADATA_BLK_SIZE	   ALIGN(sizeof(struct kblock_meta), ALIGNMENT)

// structure for the block metadata
struct kblock_meta {
	size_t size;
	uint8_t status;
	uint8_t id;
	struct kblock_meta *next;
	struct kblock_meta *prev;
};

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif /* !MM_KMALLOC_H */
