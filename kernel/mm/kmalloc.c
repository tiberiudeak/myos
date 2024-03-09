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
            printf("out of memory!\n");
            return 1;
        }

        printf("physical address: %x ", starting_phys_addr);
        printf("will be mapped to virtual address: %x\n", virt);

        map_page((void*)(starting_phys_addr), (void*)virt);

        pt_entry *page = get_page(virt);

        SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);
    }

    // create metadata
    metadata_blk_header = (kblock_meta*) starting_virtual_address;

    metadata_blk_header->size = (req_pages * PAGE_SIZE) - sizeof(kblock_meta);
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

void *find_best_fit(uint32_t size) {
    return NULL;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    int ret;

    if (metadata_blk_header == NULL) {
        ret = kmalloc_init(size);
        if (ret) return NULL;
    }

    return NULL;
}

void kfree(void *ptr) {
}

