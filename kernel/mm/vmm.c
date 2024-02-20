#include <mm/vmm.h>
#include <mm/pmm.h>

#include <stddef.h>

page_directory *current_page_directory = 0;
address current_pd_phy_address = 0;

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
pt_entry *get_pt_entry(page_table *pt, address virtual_address) {
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
pd_entry *get_pd_entry(page_directory *pd, address virtual_address) {
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
	void *block = kalloc(1);

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
		kfree(address, 1);
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
 * @return 0 if successful, 0 otherwise
 */
uint8_t set_page_directory(page_directory *pd) {
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
 * This function invalidates the TPL entry for the given virtual address.
 *
 * @param virtual_address The virtual address
 */
void flush_tlb_entry(address virtual_address) {
	__asm__ __volatile__ ("cli; invlpg (%0); sti" : : "r"(virtual_address));
}
