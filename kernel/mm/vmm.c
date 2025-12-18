#define pr_log_fmt(msg)	"vmm: " msg
#include <kernel/global_addresses.h>
#include <kernel/string.h>
#include <kernel/tty.h>
#include <kernel/utils.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>

#include <stddef.h>

// get page directory from the linker
extern char boot_page_directory[];

extern char _kernel_text_sec_start[];
extern char _kernel_text_sec_end[];
extern char _kernel_rodata_sec_start[];
extern char _kernel_rodata_sec_end[];

extern char _kernel_end[];

struct page_directory *current_page_directory = 0;
struct page_directory *kernel_page_directory = 0;

// vmm_bitmap for the page allocator
uint8_t vmm_bitmap[VMM_BITMAP_SIZE];
extern uint8_t pmm_bitmap[BITMAP_SIZE];

/**
 * @brief Return the PTE for the given virtual address
 *
 * @param virtual_address		The virtual address
 * @return The corresponding page table entry
 */
pt_entry *vmm_get_pte(uint32_t virtual_address) {
	uint32_t *pd = (uint32_t *) PD_VIRT_ADDR;

	// check if page directory entry is present
	if (!(pd[PAGE_DIRECTORY_INDEX(virtual_address)] & PAGE_PDE_PRESENT)) {
		return 0;
	}

	return ((pt_entry *) (PT_VIRT_BASE) +
			TABLES_PER_DIR * PAGE_DIRECTORY_INDEX(virtual_address) +
			PAGE_TABLE_INDEX(virtual_address));

	// // get current page directory
	// struct page_directory *pd = current_page_directory;

	// // get corresponding PDE for the given virtutal address
	// pd_entry *pde = &pd->entries[PAGE_DIRECTORY_INDEX(virtual_address)];

	// // get the page table
	// struct page_table *pt = (struct page_table *) PAGE_GET_PHY_ADDRESS(pde);

	// // return the corresponding PTE for the given virtual address
	// return &pt->entries[PAGE_TABLE_INDEX(virtual_address)];
}

/**
 * @brief Get the physical address corresponding to the given virtual address
 *
 * @param virtual_address	The virtual address
 * @return The physical address, or 0 if there is none
 */
uint32_t vmm_virt_to_phys(uint32_t virtual_address) {
	uint32_t *pd = (uint32_t *) PD_VIRT_ADDR;

	// check if page directory entry is present
	if (!(pd[PAGE_DIRECTORY_INDEX(virtual_address)] & PAGE_PDE_PRESENT)) {
		return 0;
	}

	pt_entry pte = *vmm_get_pte(virtual_address);

	// check if page table entry is present
	if (!(pte & PAGE_PTE_PRESENT)) {
		return 0;
	}

	return PAGE_FRAME(pte) + PAGE_OFFSET(virtual_address);
}

/**
 * @brief Allocate requested nr of pages
 *
 * Regions of virtual memory will be contiguous, but not
 * necessarily contiguous on physical memory as well
 *
 * @param nr_pages	Number of pages
 * @return virtual address of the starting page,
 * NULL otherwise
 */
void *allocate_pages(uint32_t nr_pages) {
    if (nr_pages == 0) {
        return NULL;
	}

    uint32_t first_fit_page = find_first_fit(
        vmm_bitmap, VMM_BITMAP_SIZE,
        (END_RAM - KERNEL_BASE_ADDR + 1) / PAGE_SIZE,
        nr_pages);

	// index 0 is reserved in the vmm init 2 function
    if (first_fit_page == 0) {
        return NULL;
	}

    uint32_t base_vaddr = first_fit_page * PAGE_SIZE + KERNEL_BASE_ADDR;
    uint32_t mapped_pages = 0;

    for (uint32_t i = 0; i < nr_pages; i++) {
        uint32_t page_index = first_fit_page + i;
        uint32_t vaddr = page_index * PAGE_SIZE + KERNEL_BASE_ADDR;
		uint32_t phys = (uint32_t) pmm_allocate_blocks(1);

		if (!phys) {
			goto error;
		}

        if (vmm_map_page(phys, vaddr,
            PAGE_PDE_PRESENT | PAGE_PDE_WRITABLE,
            PAGE_PTE_PRESENT | PAGE_PTE_WRITABLE)) {
			// free current allocated block of physical mem
			pmm_free_blocks((void*) phys, 1);
			goto error;
        }

        memset((void *) vaddr, 0, PAGE_SIZE);
        set_bit_in_bitmap(vmm_bitmap, VMM_BITMAP_SIZE, page_index);
        mapped_pages++;
    }

    return (void *) base_vaddr;

error:
	for (uint32_t j = 0; j < mapped_pages; j++) {
		uint32_t undo_idx = first_fit_page + j;
		uint32_t undo_vaddr = undo_idx * PAGE_SIZE + KERNEL_BASE_ADDR;
		void *phys = vmm_unmap_page(undo_vaddr);

		if (!phys) {
			printk("warning: could not unmap va = 0x%.8x\n", undo_vaddr);
		} else {
			pmm_free_blocks(phys, 1);
		}
		unset_bit_in_bitmap(vmm_bitmap, VMM_BITMAP_SIZE, undo_idx);
	}

	return NULL;
}

/**
 * @brief Free given number of pages starting at the given address
 *
 * @param address	Virtual address
 * @param nr_pages	Number of pages
 */
void free_pages(void *address, uint32_t nr_pages) {
    if (nr_pages == 0) {
        return;
	}

    uint32_t addr = (uint32_t) address;

    if (addr < KERNEL_BASE_ADDR) {
        panic("free_pages: free page below kernel base");
    }

    if (addr % PAGE_SIZE != 0) {
        panic("free_pages: unaligned page free");
    }

    for (uint32_t i = 0; i < nr_pages; i++) {
        uint32_t vaddr = addr + i * PAGE_SIZE;
        uint32_t page_index = (vaddr - KERNEL_BASE_ADDR) / PAGE_SIZE;

        if (get_bit_from_bitmap(vmm_bitmap, VMM_BITMAP_SIZE, page_index) == 0) {
            panic("free_pages: page already free");
		}

        uint32_t phys = (uint32_t) vmm_unmap_page(vaddr);

        if (!phys) {
            printk("warning: could not unmap va = 0x%.8x\n", vaddr);
            continue;
        }

        pmm_free_blocks((void *) phys, 1);
        unset_bit_in_bitmap(vmm_bitmap, VMM_BITMAP_SIZE, page_index);
    }
}

/**
 * Reload address of page directory - cache is also cleared
 * @param pr_addr Physical address of the page directory
 */
void inline vmm_reload_cr3(uint32_t pd_addr) {
	__asm__ __volatile__("movl %%eax, %%cr3" : : "a"(pd_addr));
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
uint8_t vmm_set_page_directory(struct page_directory *pd) {
	if (pd == NULL) {
		return 1;
	}

	current_page_directory = pd;

	vmm_reload_cr3((uint32_t) current_page_directory);

	return 0;
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
	pd_entry *pde =
		&pd->entries[PAGE_DIRECTORY_INDEX((uint32_t) virtual_address)];

	// if the page directory entry is not present, create it
	if (!(*pde & PAGE_PDE_PRESENT)) {
		// allocate block for the new page table
		void *block = pmm_allocate_blocks(1);

		if (block == NULL) {
			return 1;
		}

		// clear page table
		memset(block, 0, sizeof(struct page_table));

		// set frame and present and read-write bits
		SET_FRAME(pde, (uint32_t) block);
		SET_ATTRIBUTE(pde, PAGE_PDE_PRESENT);
		SET_ATTRIBUTE(pde, PAGE_PDE_WRITABLE);
		SET_ATTRIBUTE(pde, PAGE_PDE_USER);
	} else {
		// printk("page already present in pd!\n");
	}

	// get address of the page table
	struct page_table *pt = (struct page_table *) PAGE_GET_PHY_ADDRESS(pde);

	// get corresponding PTE for the given virtual address
	pt_entry *pte = &pt->entries[PAGE_TABLE_INDEX((uint32_t) virtual_address)];

	// set frame and present bit
	SET_FRAME(pte, (uint32_t) physical_address);
	SET_ATTRIBUTE(pte, PAGE_PTE_PRESENT);

	return 0;
}

// Clear page table - the physical address of the page table is
// determined based on the given virtual address, which will get
// translated in order to get its corresponding page table, which
// will end up being cleared
void _clear_pt(uint32_t virtual_address) {
	uint32_t *pt = (uint32_t *) (PT_VIRT_BASE +
		PAGE_DIRECTORY_INDEX(virtual_address) * PAGE_SIZE);

	memset(pt, 0, PAGE_SIZE);
}

/**
 * @brief Map virtual address to physical address
 *
 * @param physical_address 	The physical address
 * @param virtual_address 	The virtual address
 * @param pde_flags			PDE flags
 * @param pte_flags			PTE flags
 *
 * @return 0 if successful, 1 otherwise
 */
int vmm_map_page(uint32_t physical_address, uint32_t virtual_address,
		PAGE_PDE_FLAGS pde_flags, PAGE_PTE_FLAGS pte_flags) {
	if (virtual_address % PAGE_SIZE != 0) {
		panic("virtual address not page aligned");
	}

	// get current page directory
	struct page_directory *pd = current_page_directory;

	// get corresponding PDE for the given virtual address
	pd_entry *pde =
		&pd->entries[PAGE_DIRECTORY_INDEX(virtual_address)];

	// if the page directory entry is not present, create it
	if (!(*pde & PAGE_PDE_PRESENT)) {
		// allocate block for the new page table
		void *block = pmm_allocate_blocks(1);

		if (block == NULL) {
			return 1;
		}

		// set frame and flags
		*pde = 0;
		SET_FRAME(pde, (uint32_t) block);
		SET_ATTRIBUTE(pde, pde_flags);
		_clear_pt(virtual_address);
	}

	// get corresponding PTE for the given virtual address
	pt_entry *pte = vmm_get_pte(virtual_address);

	// set frame and flags
	*pte = 0;
	SET_FRAME(pte, physical_address);
	SET_ATTRIBUTE(pte, pte_flags);

	// reload %cr3
	vmm_reload_cr3(vmm_virt_to_phys((uint32_t) pd));

	return 0;
}

void *vmm_map_page_phys(uint32_t physical_address, PAGE_PDE_FLAGS pde_flags,
		PAGE_PTE_FLAGS pte_flags, uint32_t size) {
	uint32_t req_pages = size / PAGE_SIZE;

	if (size % PAGE_SIZE > 0) {
		req_pages++;
	}

    uint32_t first_fit_page = find_first_fit(
        vmm_bitmap, VMM_BITMAP_SIZE,
        (END_RAM - KERNEL_BASE_ADDR + 1) / PAGE_SIZE,
        req_pages);

	if (first_fit_page == 0) {
		return NULL;
	}

    uint32_t base_vaddr = first_fit_page * PAGE_SIZE + KERNEL_BASE_ADDR;
    uint32_t mapped_pages = 0;

    for (uint32_t i = 0; i < req_pages; i++) {
        uint32_t page_index = first_fit_page + i;
        uint32_t vaddr = page_index * PAGE_SIZE + KERNEL_BASE_ADDR;
		uint32_t phys =  physical_address + i * BLOCK_SIZE;

        if (vmm_map_page(phys, vaddr, pde_flags, pte_flags)) {
			goto error;
        }

        set_bit_in_bitmap(vmm_bitmap, VMM_BITMAP_SIZE, page_index);
        mapped_pages++;
    }

	return (void *) base_vaddr;

error:
	for (uint32_t j = 0; j < mapped_pages; j++) {
		uint32_t undo_idx = first_fit_page + j;
		uint32_t undo_vaddr = undo_idx * PAGE_SIZE + KERNEL_BASE_ADDR;
		if (!vmm_unmap_page(undo_vaddr)) {
			printk("warning: page could not be unmapped: vaddr = 0x%.8x\n", undo_vaddr);
		}
		unset_bit_in_bitmap(vmm_bitmap, VMM_BITMAP_SIZE, undo_idx);
	}

	return NULL;
}

/**
 * @brief Unmap the page for the given virtual address
 *
 * @param virtual_address The virtual address
 * @return physical address
 */
void *vmm_unmap_page(uint32_t virtual_address) {
	// get page table entry
	pt_entry *pte = vmm_get_pte((uint32_t) virtual_address);
	uint32_t paddr;

	if (!pte) {
		return NULL;
	}

	paddr = PAGE_FRAME(virtual_address);

	// set frame to address 0 and clear present bit
	SET_FRAME(pte, 0);
	CLEAR_ATTRIBUTE(pte, PAGE_PTE_PRESENT);
	vmm_reload_cr3(vmm_virt_to_phys((uint32_t) PD_VIRT_ADDR));

	return (void *) paddr;
}

void vmm_unmap_page_phys(uint32_t virtual_address, uint32_t size) {
	uint32_t pages = size / PAGE_SIZE;

	if (size % PAGE_SIZE > 0) {
		pages++;
	}

	for (uint32_t i = 0; i < pages; i++) {
		if (!vmm_unmap_page(virtual_address + i * PAGE_SIZE)) {
			printk("warning: page could not be unmapped: vaddr = 0x%.8x\n",
					virtual_address + i * PAGE_SIZE);
		}
	}
}

/**
 * @brief Second phase in setting up the virtual memory
 *
 * - unmap identity map of the first 4MB
 * - set R/W bit in the page table entries based on section
 * - reload %cr3
 * - init the page allocator
 *
 * @return 0 if successful, 1 otherwise
 */
int vmm_init_phase2(void) {
	struct page_directory *pd = (struct page_directory *) boot_page_directory;
	current_page_directory = pd;

	// unset R/W bit for the entries corresponding to .text
	// starting address should be already page aligned from the linker
	for (uint32_t start = (uint32_t) _kernel_text_sec_start;
				start < ALIGN((uint32_t) _kernel_text_sec_end, PAGE_SIZE);
				start += PAGE_SIZE) {
		pt_entry *pte = vmm_get_pte(start);
		CLEAR_ATTRIBUTE(pte, PAGE_PTE_WRITABLE);
	}

	// unset R/W bit for the entries corresponding to .rodata
	// starting address should be already page aligned from the linker
	for (uint32_t start = (uint32_t) _kernel_rodata_sec_start;
				start < ALIGN((uint32_t) _kernel_rodata_sec_end, PAGE_SIZE);
				start += PAGE_SIZE) {
		pt_entry *pte = vmm_get_pte(start);
		CLEAR_ATTRIBUTE(pte, PAGE_PTE_WRITABLE);
	}

	// unmap identity map of the first 4MB as it's no longer needed
	pd->entries[0] = 0;

	// reload page directory addr into %cr3
	vmm_reload_cr3(vmm_virt_to_phys((uint32_t) pd));
	kernel_page_directory = current_page_directory;

	// init the page allocator vmm_bitmap
	return mark_region_reserved(vmm_bitmap, VMM_BITMAP_SIZE, KERNEL_BASE_ADDR,
			(uint32_t) _kernel_end - KERNEL_BASE_ADDR, KERNEL_BASE_ADDR / PAGE_SIZE);
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
	struct page_directory *dir = pmm_allocate_blocks(1);

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
	// memset(dir + 1, 0, sizeof(pd_entry) *
	// PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR) - 1);
	for (uint32_t i = 1; i < PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR); i++) {
		dir->entries[i] = 0;
	}

	return dir;
}

/**
 * @brief Set initial kernel virtual address space as current address space
 *
 * This function restores the initial kenel virtual address space as the current
 * address space, while also deallocating the used memory for the previous
 * virtual address space (except the kernel mappings).
 */
void restore_kernel_address_space(void) {
	int ret;

	// deallocate potential memory for the current_page_directory:
	// go through the current page directory and deallocate all page
	// tables except the ones for kernel: exclude the first 4MB (that is
	// why the index starts at 1) and the memory above 0xC0000000
	for (uint32_t i = 1; i < PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR); i++) {
		if ((uint32_t) current_page_directory->entries[i] != 0) {
			pd_entry phys_address_of_page_table =
				current_page_directory->entries[i];

			pmm_free_blocks(
				(void *) PAGE_GET_PHY_ADDRESS(&phys_address_of_page_table), 1);
		}
	}

	struct page_directory *tmp = current_page_directory;

	// set kernel page directory to current page directory
	ret = vmm_set_page_directory(kernel_page_directory);

	if (ret) {
		printk("failed to change page directory!\n");
		__asm__ __volatile__("cli; hlt");
	}

	// free memory with the old page directory
	pmm_free_blocks((void *) tmp, 1);
}

/**
 * @brief Free physical memory used by the current process
 *
 * This function goes through the current page directory and searches for
 * entries that are not empty (and are not made by the kernel). If one such page
 * directory entry is found, then the function goes through the corresponding
 * page table entries and frees those pages.
 */
void free_proc_phys_mem(void) {
	for (uint32_t i = 1; i < PAGE_DIRECTORY_INDEX(KERNEL_VIRT_ADDR); i++) {
		if ((uint32_t) current_page_directory->entries[i] != 0) {
			pd_entry pde = current_page_directory->entries[i];

			// get page table corresponding to the pde
			struct page_table *pt =
				(struct page_table *) PAGE_GET_PHY_ADDRESS(&pde);
			// printk("freeing %x %d\n", pt, i);

			for (uint32_t j = 0; j < 1024; j++) {
				if (pt->entries[j] != 0) {
					// printk("freeing phys mem: %x, %d\n",
					// PAGE_GET_PHY_ADDRESS(&pt->entries[j]), j);
					free_pages(&pt->entries[j], 1);
				}
			}
		}
	}
}

/**
 * @brief Set the page directory to the kernel page directory
 * @return 1 if error occured, 0 otherwise
 */
uint8_t set_kernel_page_directory(void) {
	return vmm_set_page_directory(kernel_page_directory);
}

/**
 * @brief Map given physical address to a virtual address
 * - should be called before the physical and virtual
 *   memory allocators have been initialized, the normal
 *   vmm_map_page should be used afterwards
 *
 * The function will do the mapping between the given physical
 * address and a virtual address of this form:
 *
 * 0xFFC<PT_index><offset>
 *
 * - the page directory index will be 1023 (the last entry)
 * - because the last entry in the PD points to itself, the
 *   PD_index will be used as the index in the PD as well
 *   (instead of a page table in the normal case)
 * - as a result, the PD will be used as PD and PT for
 *   addresses of this form
 *
 * @param physical_address	The physical address to be mapped
 * @return the virtual address corresponding to the given
 * physical address if any could be obtained, 0 otherwise
 */
uint32_t vmm_map_page_early(uint32_t physical_address) {
	uint32_t *pd = (uint32_t *) PD_VIRT_ADDR;
	uint32_t index = 0;

	for (; index < TABLES_PER_DIR; index++) {
		if (*(pd + index) & PAGE_PTE_PRESENT) {
			continue;
		}

		// found a free PTE
		uint32_t *pte = pd + index;
		memset(pte, 0, sizeof(uint32_t));
		SET_FRAME(pte, physical_address);
		SET_ATTRIBUTE(pte, PAGE_PTE_PRESENT | PAGE_PTE_WRITABLE);

		uint32_t vaddr = PT_VIRT_BASE + index * PAGE_SIZE + PAGE_OFFSET(physical_address);
		vmm_reload_cr3(vmm_virt_to_phys((uint32_t) pd));
		return vaddr;
	}

	return 0;
}

/**
 * @brief Map video memory
 *
 * The corresponding region will start with the first aligned
 * address after the kernel
 *
 * @param phys_addr		Starting physical address of the video region
 * @param size			Region size
 * @return virtual address of video mem, 0 if errors occurred
 */
uint32_t vmm_map_video_mem(uint32_t phys_addr, uint32_t size) {
	// kernel_end should be already aligned at this point, but this doesn't harm
	uint32_t vaddr = ALIGN((uint32_t) _kernel_end, PAGE_SIZE);
	uint32_t vaddr_copy = vaddr;

	// number of pages
	uint32_t nr_pages = size / PAGE_SIZE;

	if (size % PAGE_SIZE) {
		nr_pages++;
	}

	// map every page
	for (uint32_t i = 0; i < nr_pages; i++, vaddr += PAGE_SIZE, phys_addr += BLOCK_SIZE) {
		if (vmm_map_page(phys_addr, vaddr,
				PAGE_PDE_PRESENT | PAGE_PDE_WRITABLE,
				PAGE_PTE_PRESENT | PAGE_PDE_WRITABLE)) {
			// no unmapping as we cannot recover from this
			return 0;
		}
	}

	// mark region reserved in the page allocator and physical mem allocator
	if (mark_region_reserved(vmm_bitmap, VMM_BITMAP_SIZE, vaddr_copy, size,
				KERNEL_BASE_ADDR / PAGE_SIZE) ||
		mark_region_reserved(pmm_bitmap, BITMAP_SIZE, phys_addr, size, 0)) {
		// no unmapping as we cannot recover from this
		return 0;
	}

	return vaddr_copy;
}
