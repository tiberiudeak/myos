#ifndef MM_KMALLOC_H
#define MM_KMALLOC_H 1

#include <stddef.h>
#include <stdint.h>

#define STATUS_FREE             0
#define STATUS_ALLOC            1

#define ALIGNMENT               8
//#define ALIGN(size)           (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
//#define ALIGN_TO_PAGE(size)   (((size) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))
#define ALIGN(size, alignment)  (((size) + (alignment - 1)) & ~(alignment - 1))
#define METADATA_BLK_SIZE       ALIGN(sizeof(kblock_meta), ALIGNMENT)

// structure for the block metadata
typedef struct kblock_meta {
    size_t size;
    uint8_t status;
    struct kblock_meta *next;
    struct kblock_meta *prev;
} kblock_meta;

void *kmalloc(size_t size);
void kfree(void *ptr);
void kmalloc_print_list(void);

#endif /* !MM_KMALLOC_H */

