#ifndef MM_KMALLOC_H
#define MM_KMALLOC_H 1

#include <stddef.h>

#define STATUS_FREE         0
#define STATUS_ALLOC        1

#define PAGE_SIZE           4096
#define ALIGNMENT           8
#define ALIGN(size)         (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// structure for the block metadata
typedef struct {
    size_t size;
    int status;
    struct kblock_meta *next;
    struct kblock_meta *prev;
} kblock_meta;

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif /* !MM_KMALLOC_H */

