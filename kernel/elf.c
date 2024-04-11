#include <kernel/tty.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <fs.h>
#include <elf.h>

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

elf_phys_mem_info *elf_phys_mem_info_header = NULL;

/**
 * @brief Add new node with the given info to list
 */
void add_phys_info(void *addr, void *virt_addr, uint32_t num_blocks) {
    elf_phys_mem_info *tmp = kmalloc(sizeof(elf_phys_mem_info));

    if (tmp == NULL) {
        printk("out of memory\n");
        return;
    }

    tmp->num_blocks = num_blocks;
    tmp->physical_address = addr;
    tmp->virtual_address = virt_addr;
    tmp->next = NULL;

    if (elf_phys_mem_info_header == NULL) {
        elf_phys_mem_info_header = tmp;
        return;
    }

    elf_phys_mem_info *tmp2 = elf_phys_mem_info_header;

    while (tmp2->next != NULL)
        tmp2 = (elf_phys_mem_info*)tmp2->next;

    tmp2->next = (struct elf_phys_mem_info*)tmp;
}

void deallocate_elf_memory(void) {
    elf_phys_mem_info *tmp = elf_phys_mem_info_header;

    while (tmp != NULL) {
        // free physical memory
        free_blocks(tmp->physical_address, tmp->num_blocks);
        tmp = (elf_phys_mem_info*) tmp->next;

        // unmap pages
        unmap_page(tmp->virtual_address);
    }

    // free list
    elf_phys_mem_info *tmp2;
    tmp = elf_phys_mem_info_header;

    while (tmp != NULL) {
        tmp2 = tmp;
        tmp = (elf_phys_mem_info*) tmp->next;
        kfree(tmp2);
    }

    elf_phys_mem_info_header = NULL;
}

/**
 * @brief Check if file at given address is an ELF file
 *
 * This function searches for the ELF magic number that is supposed to
 * be at the beginning of the file and also checks if the ELF file is
 * for 32-bit.
 *
 * Also check if ELF type is EXEC (only EXEC type supported for now)
 *
 * @elf_address Address of the file
 *
 * @return 1 if error occured, 0 otherwise
 */
uint32_t check_elf(uint32_t *elf_address) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*) elf_address;

    // check magic
    if (elf_header->e_ident[0] != 0x7F || elf_header->e_ident[1] != 'E' ||
            elf_header->e_ident[2] != 'L' || elf_header->e_ident[3] != 'F')
        return 1;

    
    // check if it's for 32 bits
    if (elf_header->e_ident[4] != 1)
        return 1;

    // check if type is EXEC (only EXEC types supported for now)
    if (elf_header->e_type != ET_EXEC)
        return 1;

    return 0;
}

void print_elf_header(Elf32_Ehdr *header) {
    printk("\nELF Header:\n");

    printk("\tType: %s\n", header->e_type == ET_NONE ? "" : header->e_type == ET_REL ?
            "REL (Relocatable file)" : header->e_type == ET_EXEC ? "EXEC (Executable file)" :
            header->e_type == ET_DYN ? "DYN (Dynamic file)" : "CORE (Core file)");
    printk("\tMachine: %s\n", header->e_machine == 3 ? "Intel 80386" : "Unknown");
    printk("\tVersion %d\n", header->e_version);
    printk("\tEntry point address: %x\n", header->e_entry);
    printk("\tStart of program headers: %d (bytes into file)\n", header->e_phoff);
    printk("\tStart of section headers: %d (bytes into file)\n", header->e_shoff);
    printk("\tFlags: %x\n", header->e_flags);
    printk("\tSize of this header: %d (bytes)\n", header->e_ehsize);

    printk("\tSize if program headers: %d (bytes)\n", header->e_phentsize);
    printk("\tNumber of program headers: %d\n", header->e_phnum);

    printk("\tSize of section headers: %d (bytes)\n", header->e_shentsize);
    printk("\tNumber of section headers: %d\n", header->e_shnum);
    printk("\tSection header string table index: %d\n", header->e_shstrndx);
}

void *load_elf(uint32_t *elf_address) {
    
    // read from loaded file and create another location in the main memory
    // to put the data and code segments
    Elf32_Ehdr *elf_header = (Elf32_Ehdr*) elf_address;

    // print_elf_header(elf_header);

    // read elf header
    Elf32_Phdr *pr_header = NULL;

    // printk("Program headers info:\n");
    for (size_t i = 0; i < elf_header->e_phnum; i++) {
        pr_header = (Elf32_Phdr*) ((void*)elf_address + elf_header->e_phoff) + i;

        if (pr_header->p_type != PT_LOAD)
            continue;

        // printk("\tSegment number: %d\n", i + 1);
        // printk("\tType: %s\n", pr_header->p_type == PT_LOAD ? "Loadable program segment" :
        //         pr_header->p_type == PT_DYNAMIC ? "Dynamic linking information" : "Unknown");
        // printk("\tOffset: %d\n", pr_header->p_offset);

        // printk("\tSegment virtual address: %x\n", pr_header->p_vaddr);
        // printk("\tSegment physical address: %x\n", pr_header->p_paddr);
        // printk("\tSegment size in file: %d (bytes)\n", pr_header->p_filesz);
        // printk("\tSegment size in memory: %d (bytes)\n", pr_header->p_memsz);
        // printk("\tFlags: %x\n", pr_header->p_flags);
        // printk("\tSegment alignment: %x\n", pr_header->p_align);
        // printk("\n");

        // each segment will have minimum a block of memory
        uint8_t needed_blocks = 0;

        if (pr_header->p_memsz % BLOCK_SIZE == 0) {
            needed_blocks = pr_header->p_memsz / BLOCK_SIZE;
        }
        else {
            needed_blocks = pr_header->p_memsz / BLOCK_SIZE + 1;
        }

        // printk("segment %d needed %d blocks\n", i, needed_blocks);

        // map obtained blocks to corresponding virtual addresses
        for (uint32_t i = 0, virt = pr_header->p_vaddr; i < needed_blocks; i++, virt += PAGE_SIZE) {
            void *addr = allocate_blocks(needed_blocks);

            if (addr == NULL) {
                printk("out of memory!\n");
                return NULL;
            }

            map_page(addr, (void*) virt);

            pt_entry *page = get_page(virt);

            SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);

            add_phys_info(addr, (void*)virt, needed_blocks);
        }

        // copy data from the ELF file image to the "executable location"
        uint32_t *src, *dst;
        uint32_t length = pr_header->p_memsz;

        src = (void*) elf_address + pr_header->p_offset;
        dst = (void*) pr_header->p_vaddr;

        memcpy(dst, src, length);
    }

    // return entry point to that location
    // printk("info about the executable:\n");
    // printk("entry point: %x\n", elf_header->e_entry);

    return (void*)elf_header->e_entry;
}

int32_t execute_elf(char *name) {
    // data from the kernel
    extern open_files_table_t *open_files_table;
    int ret;

    // open syscall to get the fd for the elf file
    int fd = open(name, O_RDONLY);

    if (fd < 0)
        return 1;

    open_files_table_t *oft_entry = open_files_table + fd;

    ret = check_elf(oft_entry->address);

    if (ret)
        goto err;

    // get entry point of elf
    void *entry_point = load_elf(oft_entry->address);

    // close file descriptor
    ret = close(fd);

    if (ret)
        return 1;

    int32_t (*program)(int argc, char *argv[]);
    program = (int32_t (*)(int, char**)) entry_point;

    // start program execution
    int32_t return_code = program(1, NULL);

    // deallocate memory
    deallocate_elf_memory();

    return return_code;

err:
    deallocate_elf_memory();
    close(fd);
    return 1;
}

