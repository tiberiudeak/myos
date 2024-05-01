#include <mm/kmalloc.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <kernel/tty.h>

#include <string.h>
#include <global_addresses.h>

extern char kernel_end[];           // symbol from the kernel linker script
kblock_meta *metadata_blk_header;

// starting virtual address will be the starting virtual address of the kernel (which
// is 0xC0000000) plus the total size of the kernel, rounded up to the nearest page
// alligned address -> done in the init() function
uint32_t starting_virtual_address = 0;
uint32_t current_virtual_address = 0;

/**
 * @brief Called for the first kmalloc(), initializes the metadata structure
 *
 * This function requests physical memory from the physical memory manager to cover the
 * requested size and maps it to virtual address. The starting virtual address (kernel heap)
 * is determined as described above. After this, the metadata block header is populated.
 *
 * @param size Requested size
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t kmalloc_init(size_t size) {
    // determine starting virtual address
    uint32_t kernel_size_bytes = (uint32_t) (kernel_end - KERNEL_ADDRESS);
    starting_virtual_address = KERNEL_VIRT_ADDR + ALIGN(kernel_size_bytes, PAGE_SIZE);

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

        // printk("physical address: %x ", starting_phys_addr);
        // printk("will be mapped to virtual address: %x\n", virt);

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

    // printk("initial metadata block:\n");
    // printk("\tsize: %d\n", metadata_blk_header->size);
    // printk("\tstatus: %d\n", metadata_blk_header->status);
    // printk("\tstarting virtual address: %x\n", (void*)metadata_blk_header + METADATA_BLK_SIZE);

    return 0;
}

/**
 * @brief Print list
 *
 * This function prints the current list
 */
void kmalloc_print_list(void) {
    kblock_meta *current = metadata_blk_header;

    while (current != NULL) {
        printk("node size: %d, status %d\n", current->size, current->status);
        current = (kblock_meta *)current->next;
    }
}

/**
 * @brief Find best block that fits the requested size
 *
 * This function goes through the list and searches for the block with the smallest available size
 * that fits the requested size.
 *
 * @param size The requested size
 *
 * @return The block address if one is found, NULL otherwise
 */
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

/**
 * @brief Split the given block in two according to the given size
 *
 * This function splits the given block in a block that is allocated and has the given
 * size and a free block that has the remaining size from the initial block.
 *
 * @param block Pointer to the block that is to be split
 * @param size  The size that the allocated block has to have
 *
 * @return The address of the block that is allocated (same as the parameter)
 */
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

/**
 * @brief Increases the heap
 *
 * This function is called when there is not sufficient space on the heap for an incoming
 * allocation request. The missing size is determined and a number of blocks to cover it is
 * requested from the physical memory allocator. The physical addresses are mapped to the
 * corresponding virtual addresses. Now, if the last node in the list is free, then its size
 * is increased with the new size and it is split if possible. If the node is not free, then
 * a new node is appended to the list and it is split if possible.
 *
 * @param size The requested size
 *
 * @return Pointer to the node in the list that accommodated the requested size
 */
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
            printk("out of memory!\n");
            return NULL;
        }

        // printk("physical address: %x ", starting_phys_addr);
        // printk("will be mapped to virtual address: %x\n", virt);

        map_page((void*)(starting_phys_addr), (void*)virt);

        pt_entry *page = get_page(virt);

        SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);

        current_virtual_address = virt + PAGE_SIZE;
    }


    // if last block is free
    if (current->status == STATUS_FREE) {
        current->size += (req_pages * PAGE_SIZE);

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

/**
 * @brief Allocate dynamic memory (called by the kernel)
 *
 * This function initializes the memory metadata structure if necessary and searches for the
 * best fit for the requested size. If a best fit is found, then the block is split if possible.
 * If one is not found, then the memory is expanded.
 *
 * @param size The requested size in bytes
 *
 * @return Starting virtual address
 */
void *kmalloc(size_t size) {
    
    if (size == 0) return NULL;

    int ret;

    // initialize metadata if necessary
    if (metadata_blk_header == NULL) {
        ret = kmalloc_init(size);
        if (ret) return NULL;
    }

    // find best fit
    kblock_meta *best_fit = (kblock_meta*) kmalloc_find_best_fit(size);

    if (best_fit != NULL) {
        // split block if there is place for at least 8 bytes + sizeof metadata block
        if (best_fit->size - ALIGN(size, ALIGNMENT) >= METADATA_BLK_SIZE + ALIGN(1, ALIGNMENT)) {
            kblock_meta *split_block = kmalloc_split_block(best_fit, size);

            split_block->status = STATUS_ALLOC;

            return (void*) split_block + METADATA_BLK_SIZE;
        }

        best_fit->status = STATUS_ALLOC;
        return (void*)best_fit + METADATA_BLK_SIZE;
    }

    // expand memory
    kblock_meta *block = kmalloc_expand_memory(size);

    if (block == NULL) return NULL;

    block->status = STATUS_ALLOC;

    return (void*)block + METADATA_BLK_SIZE;
}

/**
 * @brief Free dynamic memory (called by the kernel)
 *
 * This function goes through the list and searches for the given virtual address.
 * If it is found, the status of the block is changed to STATUS_FREE and the block
 * is coalesced with its predecessor and successor (if possible)
 *
 * @param ptr   Virtual address
 */
void kfree(void *ptr) {
    if (ptr == NULL) return;

    kblock_meta *current = metadata_blk_header;

    while (current != NULL) {
        if ((void*)current + METADATA_BLK_SIZE == ptr) {
            current->status = STATUS_FREE;

#ifdef CONFIG_READ_AFTER_FREE_PROT
            // fill memory with zeros
            memset((void*)current + METADATA_BLK_SIZE, 0, current->size);
#endif

            // coalesce if possible
            if (current->prev != NULL && current->prev->status == STATUS_FREE &&
                current->next != NULL && current->next->status == STATUS_FREE) {
                current->prev->size += current->size + current->next->size + 2 * METADATA_BLK_SIZE;
                current->prev->next = current->next->next;

                if (current->next->next != NULL) {
                    current->next->next->prev = current->prev;
                }

                return;
            }

            if (current->prev != NULL && current->prev->status == STATUS_FREE) {
                current->prev->size += current->size + METADATA_BLK_SIZE;
                current->prev->next = current->next;

                if (current->next != NULL) {
                    current->next->prev = current->prev;
                }

                return;
            }

            if (current->next != NULL && current->next->status == STATUS_FREE) {
                current->size += current->next->size + METADATA_BLK_SIZE;

                if (current->next->next != NULL) {
                    current->next->next->prev = current;
                }
                current->next = current->next->next;

                return;
            }
            
            return;
        }

        current = (kblock_meta*)current->next;
    }

    /*
     * TODO: also free physical memory when possible: detect if an entire block of memory is free
     * (meaning that there is a node in the list that has STATUS_FREE and its size is
     * PAGE_SIZE - METADATA_BLK_SIZE. I think this is only possible for the last node (as memory
     * has to be contiguous). Get physical address from virtual address:
     * page_table_entry = get_page(virtual_address)
     * physical_address = page_table_entry & 0x7FFFF000
     *
     * Make sure also to modify current_virtual_address.
     */
}

