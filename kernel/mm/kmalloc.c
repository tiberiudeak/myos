#include "include/mm/kmalloc.h"
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <global_addresses.h>

#include <stdio.h>

extern char kernel_end[];
kblock_meta *metadata_blk_header;

// starting virtual address will be the starting virtual address of the kernel (which
// is 0xC0000000) plus the total size of the kernel, rounded up to the nearest page
// alligned address -> done in the init() function
uint32_t starting_virtual_address = 0;
uint32_t current_virtual_address = 0;

uint8_t kmalloc_init(size_t size) {
    // determine starting virtual address
    uint32_t kernel_size_bytes = (uint32_t) (kernel_end - KERNEL_ADDRESS);
    starting_virtual_address = 0xC0000000 + ALIGN(kernel_size_bytes, PAGE_SIZE);

    // get necessary number of pages
    uint32_t req_pages = size / PAGE_SIZE;

    if (size % PAGE_SIZE > 0) req_pages++;

    // map physical to virtual pages and make page writeable
    for (uint32_t i = 0, virt = starting_virtual_address; i < req_pages; i++, virt += PAGE_SIZE) {
        // request pages from the physical memory manager
        uint32_t starting_phys_addr = (uint32_t) allocate_blocks(1);

        if ((void*)starting_phys_addr == NULL) {
            return 1;
        }

        printf("physical address: %x ", starting_phys_addr);
        printf("will be mapped to virtual address: %x\n", virt);

        map_page((void*)(starting_phys_addr), (void*)virt);

        pt_entry *page = get_page(virt);

        SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);

        // current available virtual address
        current_virtual_address = virt + PAGE_SIZE;
    }

    // create metadata
    metadata_blk_header = (kblock_meta*) starting_virtual_address;

    metadata_blk_header->size = (req_pages * PAGE_SIZE) - METADATA_BLK_SIZE;
    metadata_blk_header->status = STATUS_FREE;
    metadata_blk_header->next = NULL;
    metadata_blk_header->prev = NULL;

    printf("initial metadata block:\n");
    printf("\tsize: %d\n", metadata_blk_header->size);
    printf("\tstatus: %d\n", metadata_blk_header->status);
    printf("\tstarting virtual address: %x\n", (void*)metadata_blk_header + METADATA_BLK_SIZE);
    printf("sizeof metadata struct block: %d\n", METADATA_BLK_SIZE);

    return 0;
}

void kmalloc_print_list(void) {
    kblock_meta *current = metadata_blk_header;

    while (current != NULL) {
        printf("node size: %d, status %d\n", current->size, current->status);
        current = (kblock_meta*)current->next;
    }
}

void *kmalloc_find_best_fit(uint32_t size) {
    kblock_meta *current = metadata_blk_header;
    kblock_meta *best_fit = NULL;

    uint32_t min = 0xFFFFFFFF;

    while (current != NULL) {
        if (current->status == STATUS_FREE && current->size >= size) {
            if (current->size < min) {
                min = current->size;
                best_fit = current;
            }
        }

        current = (kblock_meta*)current->next;
    }

    return best_fit;
}

void *kmalloc_split_block(kblock_meta *block, uint32_t size) {
    kblock_meta *new_block = (void*)block + METADATA_BLK_SIZE + ALIGN(size, ALIGNMENT);

    new_block->size = block->size - ALIGN(size, ALIGNMENT) - METADATA_BLK_SIZE;
    new_block->status = STATUS_FREE;
    new_block->next = block->next;
    new_block->prev = (struct kblock_meta*)block;

    block->size = block->size - new_block->size - METADATA_BLK_SIZE;
    block->next = (struct kblock_meta*)new_block;

    return block;
}

void *kmalloc_expand_memory(uint32_t size) {
    kblock_meta *current = metadata_blk_header;

    while (current != NULL && current->next != NULL) {
        current = (kblock_meta*)current->next;
    }

    uint32_t needed_size = size;
    if (current->status == STATUS_FREE) {
        needed_size -= current->size;
    }

    // get necessary number of pages
    uint32_t req_pages = needed_size / PAGE_SIZE;

    if (needed_size % PAGE_SIZE > 0) req_pages++;

    uint32_t local_starting_virtual_address = current_virtual_address;

    // map physical to virtual pages and make page writeable
    for (uint32_t i = 0, virt = current_virtual_address; i < req_pages; i++, virt += PAGE_SIZE) {
        // request pages from the physical memory manager
        uint32_t starting_phys_addr = (uint32_t) allocate_blocks(1);

        if ((void*)starting_phys_addr == NULL) {
            printf("out of memory!\n");
            return NULL;
        }

        printf("physical address: %x ", starting_phys_addr);
        printf("will be mapped to virtual address: %x\n", virt);

        map_page((void*)(starting_phys_addr), (void*)virt);

        pt_entry *page = get_page(virt);

        SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);

        current_virtual_address = virt + PAGE_SIZE;
    }


    // if last block is free
    if (current->status == STATUS_FREE) {
        current->size += (req_pages * PAGE_SIZE);
        printf("current node free space: %d\n", current->size);

        // split block if possible
        if (current->size - ALIGN(size, ALIGNMENT) >= METADATA_BLK_SIZE + ALIGN(1, ALIGNMENT)) {
            return kmalloc_split_block(current, size);
        }

        return current;
    }

    // if last block is not free

    kblock_meta *new_block = (kblock_meta*)(local_starting_virtual_address);
    new_block->size = (req_pages * PAGE_SIZE) - METADATA_BLK_SIZE;
    new_block->status = STATUS_FREE;
    new_block->next = NULL;
    new_block->prev = (struct kblock_meta*)current;

    current->next = (struct kblock_meta*)new_block;

    if (new_block->size - ALIGN(size, ALIGNMENT) >= METADATA_BLK_SIZE + ALIGN(1, ALIGNMENT)) {
        return kmalloc_split_block(new_block, size);
    }

    return new_block;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    int ret;
    printf("kmalloc called with size: %d\n", size);

    // initialize metadata if necessary
    if (metadata_blk_header == NULL) {
        ret = kmalloc_init(size);
        if (ret) return NULL;
    }

    // find best fit
    kblock_meta *best_fit = (kblock_meta*)kmalloc_find_best_fit(size);

    if (best_fit != NULL) {
        // split block if there is place for at least 8 bytes + sizeof metadata block
        if (best_fit->size - ALIGN(size, ALIGNMENT) >= METADATA_BLK_SIZE + ALIGN(1, ALIGNMENT)) {
            kblock_meta *split_block = kmalloc_split_block(best_fit, size);

            split_block->status = STATUS_ALLOC;

            kmalloc_print_list();
            return (void*) split_block + METADATA_BLK_SIZE;
        }

        best_fit->status = STATUS_ALLOC;
        kmalloc_print_list();
        return (void*)best_fit + METADATA_BLK_SIZE;
    }

    // expand memory
    kblock_meta *block = kmalloc_expand_memory(size);

    if (block == NULL) return NULL;

    block->status = STATUS_ALLOC;

    kmalloc_print_list();
    return (void*)block + METADATA_BLK_SIZE;
}

void kfree(void *ptr) {
    if (ptr == NULL) return;

    kblock_meta *current = metadata_blk_header;

    while (current != NULL) {
        if ((void*)current + METADATA_BLK_SIZE == ptr) {
            current->status = STATUS_FREE;

            // coalesce if possible
            if (current->prev != NULL && current->prev->status == STATUS_FREE &&
                current->next != NULL && current->next->status == STATUS_FREE) {
                current->prev->size += current->size + current->next->size + 2 * METADATA_BLK_SIZE;
                current->prev->next = current->next->next;
                kmalloc_print_list();
                return;
            }

            if (current->prev != NULL && current->status == STATUS_FREE) {
                current->prev->size += current->size + METADATA_BLK_SIZE;
                current->prev->next = current->next;
                kmalloc_print_list();
                return;
            }

            if (current->next != NULL && current->next->status == STATUS_FREE) {
                current->size += current->next->size + METADATA_BLK_SIZE;
                current->next = current->next->next;
                kmalloc_print_list();
                return;
            }
            
            kmalloc_print_list();
            return;
        }

        current = (kblock_meta*)current->next;
    }
}

