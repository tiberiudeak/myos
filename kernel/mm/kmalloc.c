#include <mm/kmalloc.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#include <stdio.h>

kblock_meta *metadata_blk_header;

// first 200MB are for kernel data, text etc. Kernel heap starts at virtual
// address 0xCC800000
uint32_t starting_virtual_address = 0xCC800000;
//!!!!!!!!!! get starting address for heap using the kernel_end symbol created by
// the kernel linker script kernel.ld !!!!!!!!!!!!!!!!!!!!

void kmalloc_init(size_t size) {
    // get necessary number of pages
    uint32_t req_pages = size / PAGE_SIZE;

    if (size % PAGE_SIZE > 0) req_pages++;

    // request pages from the physical memory manager
    uint32_t starting_phys_addr = (uint32_t) allocate_blocks(req_pages);

    // map physical to virtual pages
    for (uint32_t i = 0, virt = starting_virtual_address; i < req_pages; i++, virt += PAGE_SIZE) {
        map_page((void*)(starting_phys_addr + i * PAGE_SIZE), (void*)virt);

        pt_entry *page = get_page(virt);

        SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);
    }

    // create metadata
    metadata_blk_header = (kblock_meta*) starting_virtual_address;

    metadata_blk_header->size = (req_pages * PAGE_SIZE) - sizeof(metadata_blk_header);
    metadata_blk_header->status = STATUS_FREE;
    metadata_blk_header->next = NULL;
    metadata_blk_header->prev = NULL;
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    if (metadata_blk_header == NULL)
        kmalloc_init(size);

    return NULL;
}

void kfree(void *ptr) {
}

