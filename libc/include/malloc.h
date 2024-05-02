#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <stdint.h>

#define STATUS_FREE         0
#define STATUS_ALLOC        1

#define ALIGNMENT           8
#define ALIGN(size)         (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define METADATA_BLK_SIZE   ALIGN(sizeof(block_meta))

typedef struct {
    size_t size;
    uint8_t status;
    struct block_meta *next;
    struct block_meta *prev;
} block_meta;

#endif /* !MALLOC_H */

