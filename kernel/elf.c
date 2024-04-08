#include <kernel/tty.h>
#include <fs.h>
#include <elf.h>

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

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

    print_elf_header(elf_header);

    // read elf header

    // return entry point to that location
    return NULL;
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

    // start execution from entry point


    return 0;

err:
    close(fd);
    return 1;
}

