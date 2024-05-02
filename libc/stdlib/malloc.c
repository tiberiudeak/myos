#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <malloc.h>

block_meta *metadata_blk_header;

void *malloc(size_t size) {
    void *addr = sbrk(0);
    printf("pr2 addr: %x\n", addr);

    addr = sbrk(2);
    printf("pr2 addr: %x\n", addr);

    addr = sbrk(4000);
    printf("pr2 addr: %x\n", addr);

    addr = sbrk(4000);
    printf("pr2 addr: %x\n", addr);

    return sbrk(0);
}

