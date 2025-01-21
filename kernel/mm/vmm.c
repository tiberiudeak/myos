#include <kernel/tty.h>
#include <kernel/global_addresses.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <kernel/string.h>
#include <stddef.h>

struct page_directory *current_page_directory = 0;
struct page_directory *kernel_page_directory = 0;

/**
 * @brief Get entry from page table for the given virtual address
 *
 * This function returns the address of the 4KB page frame from the
 * page table that corresponds to the given virtual address (the
 * least significant 12 bits are the flags)
 *
 * @param pt 				Pointer to the page table structure
 * @param virtual_address	The virtual address
 *
 * @return Address of 4KB page frame corresponding to the virtual address
 */
pt_entry *get_pt_entry(struct page_table *pt, address virtual_address) {
	if (pt == NULL) {
		return NULL;
	}

	return &pt->entries[PAGE_TABLE_INDEX(virtual_address)];
}

/**
 * @brief Get entry from page directory for the given virtual address
 *
 * This function returns the address of the page table from the entry
 * in the page directory that corresponds to the given virtual address
 * (the 12 least significant bits are the flags and that is why the
 * PAGE_TABLE_INDEX macro is used here as well)
 *
 * @param pd				Pointer to the page directory structure
 * @param virtual_address 	The virtual address
 *
 * @return Address of the corresponding page table
 */
pd_entry *get_pd_entry(struct page_directory *pd, address virtual_address) {
	if (pd == NULL) {
		return NULL;
	}

	return &pd->entries[PAGE_TABLE_INDEX(virtual_address)];
}

/**
 * @brief Allocate page given from the physical memory manager to
 * 			the given page table entry
 *
 * This function first requests one block of memory from the physical
 * memory manager and then sets the frame of the given page table entry
 * to that physical memory and also sets the page entry as present.
 *
 * @param pte Pointer to the page table entry
 *
 * @return Address allocated by the physical memory manager
 */
void *allocate_page(pt_entry *pte) {
	void *block = allocate_blocks(1);

	if (block != NULL) {
		SET_FRAME(pte, (address)block);
		SET_ATTRIBUTE(pte, PAGE_PTE_PRESENT);
	}

	return block;
}

/**
 * @brief Free physical memory "pointed" to by the given page table entry
 *
 * This function gets the physical address from the page table entry, frees
 * it and sets the present bit to 0.
 *
 * @param pte Pointer to the page table entry
 */
void free_page(pt_entry *pte) {
	void *address = (void*) PAGE_GET_PHY_ADDRESS(pte);

	if (address != NULL) {
		free_blocks(address, 1);
	}

	CLEAR_ATTRIBUTE(pte, PAGE_PTE_PRESENT);
}

/**
 * @brief Set current page directory to the given address
 *
 * This function sets the current page directory to the given address
 * and also sets it in the cr3 register (that holds the address of the
 * current page directory).
 *
 * @param pd Pointer to the page directory
 *
 * @return 0 if successful, 1 otherwise
 */
uint8_t set_page_directory(struct page_directory *pd) {
	if (pd == NULL) {
		return 1;
	}

	current_page_directory = pd;

	__asm__ __volatile__ ("movl %%eax, %%cr3" : : "a"(current_page_directory));

	return 0;
}

/**
 * @brief Flush TLB entry for the given virtual address (only in supervisor mode)
 *
 * This function invalidates the TLB entry for the given virtual address.
 *
 * @param virtual_address The virtual address
 */
void flush_tlb_entry(address virtual_address) {
	__asm__ __volatile__ ("cli; invlpg (%0); sti" : : "r"(virtual_address));
}

/**
 * @brief Map virtual address to physical address for user space
 *
 * This function maps the given virtual address to the given physical address
 * by setting the frame in the corresponding page table. See comments below
 * for more info. It sets the USER bit to 1!
 *
 * @param physical_address 	The physical address
 * @param virtual_address 	The virtual address
 *
 * @return 0 if successful, 1 otherwise
 */
uint8_t map_user_page(void *physical_address, void *virtual_address) {
	// get current page directory
	struct page_directory *pd = current_page_directory;

	// get corresponding PDE for the given virtual address
	pd_entry *pde = &pd->entries[PAGE_DIRECTORY_INDEX((uint32_t)virtual_address)];

	// if the page directory entry is not present, create it
	if (!(*pde & PAGE_PDE_PRESENT)) {
		// allocate block for the new page table
		void *block = allocate_blocks(1);

		if (block == NULL) {
			return 1;
		}

		// clear page table
		memset(block, 0, sizeof(struct page_table));

		// set frame and present and read-write bits
		SET_FRAME(pde, (uint32_t)block);
		SET_ATTRIBUTE(pde, PAGE_PDE_PRESENT);
		SET_ATTRIBUTE(pde, PAGE_PDE_WRITABLE);
		SET_ATTRIBUTE(pde, PAGE_PDE_USER);
	} else {
        //printk("page already present in pd!\n");
    }

	// get address of the page table
	struct page_table *pt = (struct page_table*)PAGE_GET_PHY_ADDRESS(pde);

	// get corresponding PTE for the given virtual address
	pt_entry *pte = &pt->entries[PAGE_TABLE_INDEX((uint32_t)virtual_address)];

	// set frame and present bit
	SET_FRAME(pte, (uint32_t)physical_address);
	SET_ATTRIBUTE(pte, PAGE_PTE_PRESENT);

	return 0;
}

/**
 * @brief Map virtual address to physical address
 *
 * This function maps the given virtual address to the given physical address
 * by setting the frame in the corresponding page table. See comments below
 * for more info.
 *
 * @param physical_address 	The physical address
 * @param virtual_address 	The virtual address
 *
 * @return 0 if successful, 1 otherwise
 */
uint8_t map_page(void *physical_address, void *virtual_address) {
	// get current page directory
	struct page_directory *pd = current_page_directory;

	// get corresponding PDE for the given virtual address
	pd_entry *pde = &pd->entries[PAGE_DIRECTORY_INDEX((uint32_t)virtual_address)];

	// if the page directory entry is not present, create it
	if (!(*pde & PAGE_PDE_PRESENT)) {
		// allocate block for the new page table
		void *block = allocate_blocks(1);

		if (block == NULL) {
			return 1;
		}

		// clear page table
		memset(block, 0, sizeof(struct page_table));

		// set frame and present and read-write bits
		SET_FRAME(pde, (uint32_t)block);
		SET_ATTRIBUTE(pde, PAGE_PDE_PRESENT);
		SET_ATTRIBUTE(pde, PAGE_PDE_WRITABLE);
	}

	// get address of the page table
	struct page_table *pt = (struct page_table*)PAGE_GET_PHY_ADDRESS(pde);

	// get corresponding PTE for the given virtual address
	pt_entry *pte = &pt->entries[PAGE_TABLE_INDEX((uint32_t)virtual_address)];

	// set frame and present bit
	SET_FRAME(pte, (uint32_t)physical_address);
	SET_ATTRIBUTE(pte, PAGE_PTE_PRESENT);

	return 0;
}

/**
 * @brief Return the PTE for the given virtual address
 *
 * This function returns the page table entry for the given virtual
 * address. More details in the code below.
 *
 * @param virtual_address The virtual address
 *
 * @return The corresponding page table entry
 */
pt_entry *get_page(address virtual_address) {
	// get current page directory
	struct page_directory *pd = current_page_directory;

	// get corresponding PDE for the given virtutal address
	pd_entry *pde = &pd->entries[PAGE_DIRECTORY_INDEX(virtual_address)];

	// get the page table
	struct page_table *pt = (struct page_table*)PAGE_GET_PHY_ADDRESS(pde);

	// return the corresponding PTE for the given virtual address
	return &pt->entries[PAGE_TABLE_INDEX(virtual_address)];
}

/**
 * @brief Unmap the page for the given virtual address
 *
 * This function gets the page table entry for the given virtual address
 * and sets the frame (so the addressof the 4KB page frame) to 0 and unsets
 * the present bit.
 *
 * @param virtual_address The virtual address
 */
void unmap_page(void *virtual_address) {
	// get page table entry
	pt_entry *pte = get_page((uint32_t)virtual_address);

	// set frame to address 0 and clear present bit
	SET_FRAME(pte, 0x0);
	CLEAR_ATTRIBUTE(pte, PAGE_PTE_PRESENT);
}

/**
 * @brief Initialize virtual memory manager
 *
 * This function creates a page directory with only two present entries: one
 * that identity maps the first 1MB of memory, and another one that maps 1MB
 * of memory starting at 0xC0000000 to the 1MB of physical memory that starts
 * at 0x00008000 (kernel location). It sets the created page directory as the
 * current page directory and enables paging. See comments below for more
 * information.
 *
 * @return 0 if successful, 1 otherwise
 */
uint8_t initialize_virtual_memory(void) {
	// allocate physical block for the page directory
	struct page_directory *pd = (struct page_directory*) allocate_blocks(1);

	if (pd == NULL) {
		return 1;
	}

	// clear all entries in the page directory
	memset(pd, 0, sizeof(struct page_directory));

	// mark each entry in the PD as read-write
	// for (uint32_t i = 0; i < 1024; i++) {
	// 	SET_ATTRIBUTE(&pd->entries[i], PAGE_PDE_WRITABLE);
	// }

	// allocate physical block for the page table that will be used
	// for the identity mapping of the first 4MB
	struct page_table *pt = (struct page_table*) allocate_blocks(1);

	if (pt == NULL) {
		return 1;
	}

	// clear all entries in the page table
	memset(pt, 0, sizeof(struct page_table));

	// allocate physical block for the page table that will be used
	// for the higher half kernel
	struct page_table *pt3gb = (struct page_table*) allocate_blocks(1);

	if (pt3gb == NULL) {
		return 1;
	}

	// clear all entries in the page table
	memset(pt3gb, 0, sizeof(struct page_table));

	// map 4MB of memory starting at 0x00000000 to the 4MB of physical memory starting
	// at 0x00000000 (identity mapping)
	for (uint32_t i = 0, block = 0x0, virt = 0x0; i < 1024; i++, block += PAGE_SIZE, virt += PAGE_SIZE) {
		// initialize page table entry to 0
		pt_entry pte = 0;

		// set writable and present bits and put the physical address in the frame
		SET_ATTRIBUTE(&pte, PAGE_PTE_PRESENT | PAGE_PTE_WRITABLE);
		SET_FRAME(&pte, block);

		// put the PTE in the page table at the corresponding entry
		pt->entries[PAGE_TABLE_INDEX(virt)] = pte;
	}

	// map 4MB of memory starting at 0xC0000000 to the 4MB of physical memory starting
	// at 0x00008000 (where the kernel resides) (higher half kernel)
	for (uint32_t i = 0, block = KERNEL_ADDRESS, virt = KERNEL_VIRT_ADDR; i < 1024; i++, block += PAGE_SIZE, virt += PAGE_SIZE) {
		//  initialize page table entry to 0
		pt_entry pte = 0;

		// set writable and present bits and put the physical address in the frame
		SET_ATTRIBUTE(&pte, PAGE_PTE_PRESENT | PAGE_PTE_WRITABLE);
		SET_FRAME(&pte, block);

		// put the PTE in the page table at the corresponding index
		pt3gb->entries[PAGE_TABLE_INDEX(virt)] = pte;
	}

	// put the pt3gb page table in the page directory at the corresponding index
	// and set the present and writable bits
	pd_entry *pde = &pd->entries[PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR)];
	SET_ATTRIBUTE(pde, PAGE_PDE_PRESENT | PAGE_PDE_WRITABLE);
	SET_FRAME(pde, (address)pt3gb);

	// put the pt page table in the page directory at the corresponding index
	// and set the present and writable bits
	pd_entry *pde2 = &pd->entries[PAGE_DIRECTORY_INDEX(0x0)];
	SET_ATTRIBUTE(pde2, PAGE_PDE_PRESENT | PAGE_PDE_WRITABLE);
	SET_FRAME(pde2, (address)pt);

	// set the page directory
	set_page_directory(pd);
    kernel_page_directory = current_page_directory;

	// enable paging
	__asm__ __volatile__ ("movl %cr0, %eax; orl $0x80000001, %eax; movl %eax, %cr0");

	return 0;
}

/**
 * @brief Create a new page directory with the kernel mappings for the first
 *      4MB and 4MB above the 0xC0000000
 * 
 * This function returns a page directory containing the kernel mappings for the
 * first 4MB of memory and the 4MB above 0xC0000000 (kernel). All other page
 * directory entries are set to 0.
 *
 * @return New page directory
 */
struct page_directory *create_address_space(void) {
    struct page_directory *dir = allocate_blocks(1);

    if (dir == NULL) {
        return NULL;
    }
#ifdef CONFIG_VERBOSE
    printk("new addr space created %x\n", dir);
#endif

	// clear all entries in the page directory
	memset(dir, 0, sizeof(struct page_directory));

    // map kernel into the virtual address space:
    // copy entries in the current page directory - what we need are only the
    // kernel pages (first 1MB and pages from 0xC0000000)
    memcpy(dir, kernel_page_directory, sizeof(pd_entry) * PAGES_PER_TABLE);

    // clear entries between the first 1MB and the higher half kernel
    // memset(dir + 1, 0, sizeof(pd_entry) * PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR) - 1);
    for (uint32_t i = 1; i < PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR); i++) {
        dir->entries[i] = 0;
    }

   return dir;
}

/**
 * @brief Set initial kernel virtual address space as current address space
 *
 * This function restores the initial kenel virtual address space as the current
 * address space, while also deallocating the used memory for the previous virtual
 * address space (except the kernel mappings).
 */
void restore_kernel_address_space(void) {
    int ret;

    // deallocate potential memory for the current_page_directory:
    // go through the current page directory and deallocate all page
    // tables except the ones for kernel: exclude the first 4MB (that is
    // why the index starts at 1) and the memory above 0xC0000000
    for (uint32_t i = 1; i < PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR); i++) {
        if ((uint32_t)current_page_directory->entries[i] != 0) {
            pd_entry phys_address_of_page_table = current_page_directory->entries[i];

            free_blocks((void*)PAGE_GET_PHY_ADDRESS(&phys_address_of_page_table), 1);
        }
    }

    struct page_directory *tmp = current_page_directory;

    // set kernel page directory to current page directory
    ret = set_page_directory(kernel_page_directory);

    if (ret) {
        printkc(4, "failed to change page directory!\n");
        __asm__ __volatile__ ("cli; hlt");
    }

    // free memory with the old page directory
    free_blocks((void*)tmp, 1);
}

/**
 * @brief Free physical memory used by the current process
 *
 * This function goes through the current page directory and searches for entries
 * that are not empty (and are not made by the kernel). If one such page directory
 * entry is found, then the function goes through the corresponding page table entries
 * and frees those pages.
 */
void free_proc_phys_mem(void) {
    for (uint32_t i = 1; i < PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR); i++) {
        if ((uint32_t)current_page_directory->entries[i] != 0) {
            pd_entry pde = current_page_directory->entries[i];

            // get page table corresponding to the pde
            struct page_table *pt = (struct page_table*)PAGE_GET_PHY_ADDRESS(&pde);
            // printk("freeing %x %d\n", pt, i);

            for (uint32_t j = 0; j < 1024; j++) {
                if (pt->entries[j] != 0) {
                    // printk("freeing phys mem: %x, %d\n", PAGE_GET_PHY_ADDRESS(&pt->entries[j]), j);
                    free_page(&pt->entries[j]);
                }
            }
        }
    }
}

/**
 * @brief Return the physical address corresponding to the given virtual address
 *
 * This funcion returns the physical address for the given virtual address based on
 * the current page directory.
 *
 * @param virt_addr The virtual address
 *
 * @return The physical address
 */
address get_physical_addr(address virt_addr) {
	// get current page directory
	struct page_directory *pd = current_page_directory;

    printk("%d - %x\n", PAGE_DIRECTORY_INDEX(virt_addr),pd->entries[PAGE_DIRECTORY_INDEX(virt_addr)]);
	// get corresponding PDE for the given virtutal address
	pd_entry *pde = &pd->entries[PAGE_DIRECTORY_INDEX(virt_addr)];

	// get the page table
	struct page_table *pt = (struct page_table*)PAGE_GET_PHY_ADDRESS(pde);

    printk("%x - %x\n", PAGE_TABLE_INDEX(virt_addr), pt->entries[PAGE_TABLE_INDEX(virt_addr)]);

	pt_entry test = pt->entries[PAGE_TABLE_INDEX(virt_addr)];

    test &= 0xFFFFF000;
    test += (virt_addr & 0x00000FFF);

    return test;
}

/**
 * @brief Set the page directory to the kernel page directory
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t set_kernel_page_directory(void) {
    return set_page_directory(kernel_page_directory);
}

