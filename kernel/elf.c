#include <arch/i386/isr.h>
#include <kernel/tty.h>
#include <kernel/shell.h>
#include <process/process.h>
#include <mm/vmm.h>
#include <mm/kmalloc.h>
#include <mm/pmm.h>
#include <global_addresses.h>
#include <fs.h>
#include <elf.h>

#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

elf_phys_mem_info *elf_phys_mem_info_header = NULL;
extern task_struct *current_running_task;

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
    free_proc_phys_mem();

    //while (tmp != NULL) {
    //    // free physical memory
    //    free_blocks(tmp->physical_address, tmp->num_blocks);
    //    tmp = (elf_phys_mem_info*) tmp->next;

    //    // unmap pages
    //    unmap_page(tmp->virtual_address);
    //}

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

uint8_t add_process_mapping(void *addr, uint32_t size) {
    mapping_t *map = kmalloc(sizeof(mapping_t));

    if (map == NULL) {
        return 1;
    }

    map->address = addr;
    map->size = size;
    map->next = NULL;

    if (current_running_task->maps == NULL) {
        current_running_task->maps = map;
        return 0;
    }

    mapping_t *tmp = current_running_task->maps;

    while (tmp->next != NULL) {
        tmp = (mapping_t*) tmp->next;
    }

    tmp->next = (struct mapping_t *) map;

    return 0;
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

void *load_elf(uint32_t *elf_address, uint32_t *ustack_start, uint32_t *ustack_end) {

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

        // virtual address not allowed for a user task
        if (pr_header->p_vaddr < LOWER_4MB_VIRT_ADDR ||
                pr_header->p_vaddr >= KERNEL_VIRT_ADDR) {
            return NULL;
        }

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

        add_process_mapping((void*) pr_header->p_vaddr, needed_blocks * BLOCK_SIZE);
        // printk("segment %d needed %d blocks\n", i, needed_blocks);

        // map obtained blocks to corresponding virtual addresses
        for (uint32_t i = 0, virt = pr_header->p_vaddr; i < needed_blocks; i++, virt += PAGE_SIZE) {
            void *addr = allocate_blocks(1);

            if (addr == NULL) {
                printk("out of memory!\n");
                return NULL;
            }

            map_user_page(addr, (void*) virt);

            pt_entry *page = get_page(virt);

            SET_ATTRIBUTE(page, PAGE_PTE_USER | PAGE_PTE_PRESENT);

            // make only those pages that need to be writable writable
            if (pr_header->p_flags & PF_W) {
                SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);
            }

            add_phys_info(addr, (void*)virt, needed_blocks);
        }

        // copy data from the ELF file image to the "executable location"
        uint32_t *src, *dst;
        uint32_t length = pr_header->p_memsz;

        src = (void*) elf_address + pr_header->p_offset;
        dst = (void*) pr_header->p_vaddr;

        memcpy(dst, src, length);

        // set up the heap and stack
        if ((int)i == elf_header->e_phnum - 1) {

            // last program header, set heap after it, initial size: 4K
            uint32_t uheap_start = ALIGN((uint32_t)pr_header->p_vaddr + needed_blocks * BLOCK_SIZE, PAGE_SIZE);
            uint32_t uheap_end = uheap_start + PAGE_SIZE;

            void *addr = allocate_blocks(1);

            if (addr == NULL) {
                printk("out of memory\n");
                return NULL;
            }

            map_user_page(addr, (void*)uheap_start);

            pt_entry *page = get_page(uheap_start);

            SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);
            SET_ATTRIBUTE(page, PAGE_PTE_USER | PAGE_PTE_PRESENT);

            add_process_mapping((void *) uheap_start, BLOCK_SIZE);
            current_running_task->heap_start = (void*) uheap_start;

            // set program break to the start of the heap
            current_running_task->program_break = (void *) uheap_start;

            printk("heap start: %x\n", uheap_start);
            printk("heap end: %x\n", uheap_end);

            // set user stack
            *ustack_end = KERNEL_VIRT_ADDR;
            *ustack_start = KERNEL_VIRT_ADDR - PAGE_SIZE;

            // map stack
            addr = allocate_blocks(1);

            if (addr == NULL) {
                printk("out of memory!\n");
                return NULL;
            }

            map_user_page(addr, (void*)(*ustack_start));

            page = get_page(*ustack_start);

            SET_ATTRIBUTE(page, PAGE_PTE_WRITABLE);
            SET_ATTRIBUTE(page, PAGE_PTE_USER | PAGE_PTE_PRESENT);

            add_phys_info(addr, (void*)(*ustack_start), 1);
            add_process_mapping((void *) (*ustack_start), BLOCK_SIZE);

            printk("uspace start: %x, end: %x, phys: %x\n", *ustack_start, *ustack_end, (uint32_t)addr);
        }
    }

    // return entry point to that location
    // printk("info about the executable:\n");
    // printk("entry point: %x\n", elf_header->e_entry);

    return (void*)elf_header->e_entry;
}

void elf_after_program_execution(int return_code) {
    deallocate_elf_memory();

    if (return_code != 0) {
        printkc(4, "execution finished with error code: %d\n", return_code);
    }
    else {
        printkc(2, "execution finished successfully\n");
    }

    shell_cleanup();
}

void set_argc_argv(uint32_t *ustack_end) {
    // populate the stack with argc and pointers to the arguments
    // the pointers will point to the heap
    uint32_t argc = current_running_task->argc;
    char **argv = current_running_task->argv;

    uint32_t *stack = (uint32_t*) *ustack_end;
    uint32_t *heap = (uint32_t*) current_running_task->heap_start;
    uint32_t *pr_break = (uint32_t*) current_running_task->program_break;

    // first populate the stack with the pointers to the arguments
    // for example, if we have 3 arguments, the stack will look like this:
    // | argc | argv[0] | argv[1] | argv[2] | ...
    // argv[0] will point to an address in the heap, that will contain the address of the first argument
    // for example, if the first argument is "hello" and there are 3 arguments, the heap will look like this:
    // | addr_arg[3] | addr_arg[2] | addr_arg[1] | "..." | "..." | "hello" |

    uint32_t *start_addr_for_strings = heap + argc;

    stack--;

    // first populate the stack with the pointers to the arguments
    for (int i = argc - 1; i >= 0; i--) {
        *stack = (uint32_t)heap;
        stack--;
        heap++;
    }

    heap = (uint32_t*) current_running_task->heap_start;

    // now populate the heap with the arguments
    for (uint32_t i = 0; i < argc; i++) {
        char *arg = argv[argc - 1 - i];
        uint32_t len = strlen(arg) + 1;
        arg[len - 1] = '\0';

        // copy the argument to the heap
        memcpy((void*) start_addr_for_strings, arg, len);

        // set the pointer to the argument at the beginning of the heap
        *heap = (uint32_t)start_addr_for_strings;

        heap++;
        start_addr_for_strings += len;

        // set the program break to the end of the last argument
        current_running_task->program_break = (void*)start_addr_for_strings;
    }

    // update the stack pointer to point to argc
    *stack = argc;
    stack--;

    *ustack_end = (uint32_t)stack;
}

uint8_t prepare_elf_execution(int argc, char **argv) {
    if (argc < 1) {
        printk("argc has to be at least 1!\n");
        return 1;
    }

    // data from the kernel
    extern open_files_table_t *open_files_table;
    int ret;

    // open syscall to get the fd for the elf file
    int fd = -1;

    __asm__ __volatile__ ("mov %0, %%ebx\n"
                        "mov %1, %%ecx\n": : "r"(argv[0]), "r"(O_RDWR));

    syscall_open();

    __asm__ __volatile ("mov %%eax, %0" : "=r"(fd));

    if (fd < 0) {
        printk("%s no such file or directory!\n", argv[0]);
        goto err2;
    }

    open_files_table_t *oft_entry = open_files_table + fd;

    ret = check_elf(oft_entry->address);

    if (ret) {
        printk("file is not an executable ELF file!\n");
        goto err;
    }

    uint32_t ustack_start = 0, ustack_end = 0;

    // get entry point of elf
    void *entry_point = load_elf(oft_entry->address, &ustack_start, &ustack_end);

    if (entry_point == NULL)
        goto err;

    // close file descriptor
    __asm__ __volatile__ ("mov %0, %%ebx" : : "r"(fd));

    syscall_close();

    __asm__ __volatile__ ("mov %%eax, %0" : "=r"(ret));

    if (ret)
        return 1;

    // update stack pointer to include the main function parameters argc and argv
    set_argc_argv(&ustack_end);

    current_running_task->context->eip = (uint32_t)entry_point;
    current_running_task->context->esp = (uint32_t)ustack_end;

    printk("eip: %x\n", current_running_task->context->eip);
    printk("esp: %x\n", current_running_task->context->esp);

    return 0;

err:
    deallocate_elf_memory();

err2:
    __asm__ __volatile__ ("mov %0, %%ebx" : : "r"(fd));

    syscall_close();

    restore_kernel_address_space();
    return 1;
}

#ifdef CONFIG_SIMPLE_SCH
int32_t execute_elf(int argc, char **argv) {
    if (argc < 1) {
        printk("argc has to be at least 1!\n");
        return 1;
    }

    // data from the kernel
    extern open_files_table_t *open_files_table;
    int ret;

    // open syscall to get the fd for the elf file
    //int fd = open(name, O_RDONLY);
    int fd = -1;

    __asm__ __volatile__ ("mov %0, %%ebx\n"
                        "mov %1, %%ecx\n": : "r"(argv[0]), "r"(O_RDWR));

    syscall_open();

    __asm__ __volatile ("mov %%eax, %0" : "=r"(fd));

    if (fd < 0) {
        printk("%s no such file or directory!\n", argv[0]);
        goto err2;
    }

    open_files_table_t *oft_entry = open_files_table + fd;

    ret = check_elf(oft_entry->address);

    if (ret) {
        printk("file is not an executable ELF file!\n");
        goto err;
    }

    uint32_t ustack_start = 0, ustack_end = 0;

    // get entry point of elf
    void *entry_point = load_elf(oft_entry->address, &ustack_start, &ustack_end);

    // close file descriptor
    __asm__ __volatile__ ("mov %0, %%ebx" : : "r"(fd));

    syscall_close();

    __asm__ __volatile__ ("mov %%eax, %0" : "=r"(ret));

    if (ret)
        return 1;

    // start program execution
    enter_usermode((uint32_t)entry_point, ustack_end);

    // deallocate memory
    // deallocate_elf_memory();

    return 0;

err:
    deallocate_elf_memory();

err2:
    __asm__ __volatile__ ("mov %0, %%ebx" : : "r"(fd));

    syscall_close();

    restore_kernel_address_space();
    return 1;
}
#endif /* CONFIG_SIMPLE_SCH */

