#ifndef MM_VMM_H
#define MM_VMM_H 1

#include <stdint.h>

#define PAGES_PER_TABLE					1024
#define TABLES_PER_DIR					1024

#define PAGE_SIZE						4096

#define PAGE_DIRECTORY_INDEX(x)         (((x) >> 22) & 0x3FF)
#define PAGE_TABLE_INDEX(x)             (((x) >> 12) & 0x3FF)
#define PAGE_GET_PHY_ADDRESS(dir_entry) ((*dir_entry) & ~0xFFF)

#define SET_ATTRIBUTE(entry, attr)      (*(entry) |= (attr))
#define CLEAR_ATTRIBUTE(entry, attr)    (*(entry) &= ~(attr))
#define TEST_ATTRIBUTE(entry, attr)     (*(entry) & (attr))
#define SET_FRAME(entry, address)       (*(entry) = (*(entry) & 0x00000FFF) | (address) & 0xFFFFF000)
#define PAGE_FRAME(addr)                ((addr) & ~0xFFF)
#define PAGE_OFFSET(addr)               ((addr) & 0xFFF)

// PD is mapped to itself into its last entry (1023rd),
// making the following virtual address point to the PD itself
#define PD_VIRT_ADDR					0xFFFFF000
// the page tables are now also accessible starting from
// this virtual address
#define PT_VIRT_BASE					0xFFC00000

#define KERNEL_BASE_ADDR				0xC0000000
#define END_RAM							0xFFFFFFFF
#define VMM_BITMAP_SIZE					((END_RAM - KERNEL_BASE_ADDR + 1) / PAGE_SIZE / 8)

/**
 * Page Directory Entry Format (4K)
 *
 *  31                  12  11  8  7    6     5    4     3     2     1    0
 * |------------------------------------------------------------------------|
 * | Address of page table | Ign | PS | Ign | A | PCD | PWT | U/S | R/W | P |
 * |------------------------------------------------------------------------|
 *
 * P: Present. If the bit is set, the page is actually in physical memory
 * R/W: 1 = page is read-write; 0 = page is read-only
 * U/S: 1 = page may be accessed by all; 0 = only the supervisor can access it
 * PWT: 1 = write-through caching; 0 = write-back
 * PCD: 1 = the page will not be cached; 0 = the page will be cached
 * A: Accessed: used to discover whether a PDE or PTE was read during the
 * virtual address translation PS: Page Size: 1 = 4MB page; 0 = 4KB page
 */
typedef enum {
	PAGE_PDE_PRESENT		= 0x1,
	PAGE_PDE_WRITABLE		= 0x2,
	PAGE_PDE_USER			= 0x4,
	PAGE_PDE_WRITE_THROUGH	= 0x8,
	PAGE_PDE_DISABLE_CACHE	= 0x10,
	PAGE_PDE_4MB			= 0x80,
	PAGE_PDE_FRAME			= 0x7FFFF000
} PAGE_PDE_FLAGS;

/**
 * Page Table Entry Format
 *
 *  31                      12  11  9  8    7    6   5    4     3     2     1 0
 * |-------------------------------------------------------------------------------|
 * | Address of 4KB page frame | Ign | G | PAT | D | A | PCD | PWT | U/S | R/W |
 * P |
 * |-------------------------------------------------------------------------------|
 *
 * See meaning of bits in the PDE format.
 * D: Dirty: used to determine whether a page has been written to
 * G: Global: tells the processor not to invalidate the TLB entry corresponding
 * 		to the page upon a MOV to CR3 instruction (change of PDE)
 * PAT: Page Attribute Table. If PAT is supported, then PAT along with PCD and
 * PWT indiccate the memory caching type. Otherwise must be set to 0 (reserved)
 */
typedef enum {
	PAGE_PTE_PRESENT		= 0x1,
	PAGE_PTE_WRITABLE		= 0x2,
	PAGE_PTE_USER			= 0x4,
	PAGE_PTE_WRITE_THROUGH	= 0x8,
	PAGE_PTE_DISABLE_CACHE	= 0x10,
	PAGE_PTE_DIRTY			= 0x40,
	PAGE_PTE_PAT			= 0x80,
	PAGE_PTE_GLOBAL			= 0x100,
	PAGE_PTE_FRAME			= 0x7FFFF000
} PAGE_PTE_FLAGS;

typedef uint32_t pd_entry;
typedef uint32_t pt_entry;

/**
 * PAge directory: array of PAGES_PER_TABLE entries
 * 1024 page tables * 4MB = 4GB
 */
struct page_directory {
	pd_entry entries[PAGES_PER_TABLE];
};

/**
 * Page table: array of PAGES_PER_TABLE entries
 * 1024 * 4096 = 4MB each
 */
struct page_table {
	pt_entry entries[TABLES_PER_DIR];
};

int vmm_init_phase2(void);
int vmm_map_page(uint32_t, uint32_t, PAGE_PDE_FLAGS, PAGE_PTE_FLAGS);
void *vmm_unmap_page(uint32_t);
pt_entry *vmm_get_pte(uint32_t);
uint32_t vmm_virt_to_phys(uint32_t);
uint32_t vmm_map_page_early(uint32_t);
void vmm_reload_cr3(uint32_t);
uint32_t vmm_map_video_mem(uint32_t, uint32_t);

uint8_t map_user_page(void *, void *);
struct page_directory *create_address_space(void);
uint8_t vmm_set_page_directory(struct page_directory *);
void restore_kernel_address_space(void);
uint8_t set_kernel_page_directory(void);
void free_proc_phys_mem(void);

void *allocate_page(void);
void free_page(void *);

#endif /* !MM_VMM_H */
