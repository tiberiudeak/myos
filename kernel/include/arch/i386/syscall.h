#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H 1

#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/isr.h>
#include <mm/vmm.h>
#include <arch/i386/pic.h>
#include <kernel/io.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <string.h>

#include <stdio.h>
#include <arch/i386/pit.h>
#include <mm/kmalloc.h>
#include <fs.h>
#include <fcntl.h>
#include <kernel/tty.h>
#include <elf.h>

#define MAX_SYSCALLS	8

/**
 * Test syscall
 */
void syscall_test0(void) {
	printk("Syscall test 0 works\n");
}

/**
 * Test syscall
 */
void syscall_test1(void) {
	printk("Syscall test 1 works\n");
}

/**
 * @brief Sleep syscall
 *
 * This function reads the number of milliseconds to wait from EBX and
 * calls wait_millis.
 */
void syscall_sleep(void) {
	uint16_t millis = 0;

	__asm__ __volatile__ ("movl %%ebx, %%ecx" : "=c"(millis) : );

	wait_millis(millis);
}

/**
 * @brief Open syscall
 *
 * the filepath will be in EBX
 * the flags will be in ECX
 */
void syscall_open(void) {
    char *path = 0;
    uint32_t flags = 0;

    // data from kernel
    extern open_files_table_t *open_files_table;

    __asm__ __volatile__ ("mov %%ebx, %0\n"
                          "mov %%ecx, %1" : "=b"(path), "=c"(flags));

    // get file inode
    inode_block_t inode = get_inode_from_path(path);

    // file doesn't exist
    if (inode.id == 0) {
        if (flags & O_CREAT) {
            inode = create_file(path);

            if (inode.id == 0) {
                printk("file could not be created\n");
                goto err;
            }
        }
        else {
            goto err;
        }
    }

    // add to open files table
    open_files_table_t *tmp_oft = open_files_table;
    int tmp_idx = 3; // skip first three entries

    // skip first three entries in the table (stdin, stdout and stderr)
    tmp_oft += 3;

    // search for an empty place for the file
    while (tmp_idx < MAX_OPEN_FILES && tmp_oft->address != 0) {
        tmp_idx++;
        tmp_oft++;
    }

    if (tmp_idx == MAX_OPEN_FILES) {
        printk("limit of open files reached: %d! close some to open more!\n", MAX_OPEN_FILES);
        goto err;
    }

    // add new open_files_table_t entry at the found free position and return the position

    // allocate memory for the inode (as the inode is a local variable here)
    void *addr = kmalloc(sizeof(inode_block_t));

    if (addr == NULL) {
        printk("out of memory\n");
        goto err;
    }

    *(inode_block_t*)addr = inode;

    tmp_oft->inode = addr;
    tmp_oft->reference_number = 0;
    tmp_oft->offset = 0;
    tmp_oft->flags = flags;

    // allocate memory for the file's data
    uint32_t needed_bytes = bytes_to_blocks(inode.size_bytes) * FS_BLOCK_SIZE;

    addr = kmalloc(needed_bytes);

    if (addr == NULL) {
        printk("out of memory\n");
        goto err;
    }

    int ret = load_file(&inode, (uint32_t)addr);

    if (ret) {
        printk("error loading block from disk\n");
        goto err;
    }

    tmp_oft->address = addr;

    // put index in the open files table into EAX and return
    __asm__ __volatile__ ("mov %%ebx, %%eax" : : "b"(tmp_idx));
    return;

err:
    __asm__ __volatile__ ("mov $-1, %eax");
}

void syscall_close(void) {
    int fd = -1;

    // get file descriptor from EBX
    __asm__ __volatile__ ("mov %%ebx, %0" : "=b"(fd));

    if (fd < 0 || fd >= MAX_OPEN_FILES) {
        goto err;
    }

    // data from kernel
    extern open_files_table_t *open_files_table;

    // get entry at fd index from the open files table
    // free the allocated memory and the entry
    open_files_table_t entry = open_files_table[fd];

    // free memory allocated for the inode
    kfree(entry.inode);

    // free memory allocated for the file's data
    kfree(entry.address);

    // empty entry
    open_files_table[fd] = (open_files_table_t){0};

    // return 0 on success
    __asm__ __volatile__ ("mov $0, %eax");
    return;

err:
    __asm__ __volatile__ ("mov $-1, %eax");
}

void syscall_read(void) {
    // data from kernel
    extern open_files_table_t *open_files_table;

    int fd = -1, read_bytes = 0;
    size_t count = 0;
    void *buf = NULL;

    //__asm__ __volatile__ ("mov %%ebx, %0\n"
    //                      "mov %%ecx, %1\n"
    //                      "mov %%edx, %2": "=b"(fd), "=c"(buf), "=d"(count)::
    //                      "edi", "esi");

    __asm__ __volatile__ ("mov %%ebx, %0": "=r"(fd));
    __asm__ __volatile__ ("mov %%ecx, %0": "=r"(buf));
    __asm__ __volatile__ ("mov %%esi, %0": "=r"(count));

    // printk("read parameters: fd: %d, count: %d, buf: %x\n", fd, count, buf);

    if (fd < 0 || fd >= MAX_OPEN_FILES || buf == NULL)
        goto err;

    if (count == 0) {
        __asm__ __volatile__ ("mov $0, %eax");
        return;
    }

    open_files_table_t *oft = open_files_table + fd;

    if (oft->address == 0)
        goto err;

    if (oft->flags & O_WRONLY)
        goto err;
    
    if (oft->inode->size_bytes < count) {
        // if number of requested bytes to read is bigger than the actual data
        read_bytes = oft->inode->size_bytes;
    }
    else {
        read_bytes = count;
    }

    // copy bytes
    memcpy(buf, oft->address, read_bytes);

    __asm__ __volatile__ ("mov %0, %%eax" : : "r"(read_bytes));

    return;

err:
    __asm__ __volatile__ ("mov $-1, %eax");
}

void syscall_write(void) {
    // data from kernel
    extern open_files_table_t *open_files_table;

    int fd, ret = 0;
    size_t count, written_bytes = 0;
    void *buf = NULL;

    __asm__ __volatile__ ("mov %%ebx, %0": "=r"(fd));
    __asm__ __volatile__ ("mov %%ecx, %0": "=r"(buf));
    __asm__ __volatile__ ("mov %%esi, %0": "=r"(count));

    // printk("write function called, fd: %d, buf: %x, count: %d\n", fd, buf, count);

    if (fd < 0 || fd >= MAX_OPEN_FILES)
        goto err;

    if (count == 0) {
        __asm__ __volatile__ ("mov $0, %eax");
        return;
    }

    // chech for special file descriptors: stdin, stdout, stderr
    if (fd == stdout || fd == stderr) {
        terminal_writestring(buf);
        written_bytes = strlen(buf);

        __asm__ __volatile__ ("mov %0, %%eax" : : "r"(written_bytes));
        return;
    }

    open_files_table_t *oft = open_files_table + fd;

    if (oft->address == 0)
        goto err;


    if (oft->flags & O_RDONLY)
        goto err;

    // TODO: add O_APPEND flag and file offset
    
    if (oft->inode->size_bytes >= count) {
        // if number of requested bytes to write is smaller than the actual data, then
        // write count bytes
        written_bytes = count;
    }
    else {
        // else write maximum oft->inode->size_bytes bytes
        // TODO: increase size of file on disk
        written_bytes = oft->inode->size_bytes;
    }

    memcpy(oft->address, buf, written_bytes);

    // update inode info
    oft->inode->size_bytes = written_bytes;
    oft->inode->size_sectors = bytes_to_sectors(written_bytes);

    // update inode info on disk TODO: fix
    //int ret = update_inode_data_disk(oft->inode);

    //printk("ret: %d\n", ret);
    //if (ret)
    //    goto err;

    // update data block on disk TODO: fix
    //ret = update_data_block_disk(oft->inode, (uint32_t)oft->address);

    //if (ret)
    //    goto err;

    __asm__ __volatile__ ("mov %0, %%eax" : : "r"(written_bytes));
    return;

err:
    __asm__ __volatile__ ("mov $-1, %eax");
}

void syscall_exit(void) {
    int return_code = -1;

    __asm__ __volatile__ ("mov %%ebx, %0" : "=r"(return_code));

    // restore kernel virtual address space
    restore_kernel_address_space();

    // cleanup elf data
    elf_after_program_execution(return_code);

    // cleanup task data
    extern task_struct *current_running_task; // data from the scheduler
    destroy_task(current_running_task);

    // return to scheduler
    simple_task_scheduler();

	// while (1) __asm__ __volatile__ ("sti; hlt; cli");
}

void *syscalls[MAX_SYSCALLS] = {
	syscall_test0,
	syscall_test1,
	syscall_sleep,
    syscall_open,
    syscall_close,
    syscall_read,
    syscall_write,
    syscall_exit
};

/**
 * @brief Syscall interrupt handler
 *
 * This is the interrupt handler for a syscall (int $0x80). Pushes
 * registers to stack, calls the syscall function in the syscall functions
 * array (with %eax * 4 as the offset), pops registers back and then returns
 * using the "iret" instruction (return from interrupt).
 *
 * __attribute__ ((naked)) means that the function doesn't need prologue/
 * epilogue sequences generated by the compiler and the only statements that
 * can be safely included are asm statements that do not have operands.
 */
__attribute__ ((naked)) void syscall_handler(void) {
	__asm__ __volatile__ (
        "cmp $8, %eax\n"	// check if syscall exists
	    										// number has to match MAX_SYSCALLS!
	    "jge syscall_invalid\n"					// if not, invalid syscall

        // save user mode state
	    "push %eax\n"
	    "push %gs\n"
	    "push %fs\n"
	    "push %es\n"
	    "push %ds\n"
	    "push %edi\n"
	    "push %esi\n"
	    "push %edx\n"
	    "push %ecx\n"
	    "push %ebx\n"
	    "push %esp\n"


        // get kernel mode state
        //"mov %eax, %ebx\n"

        //"mov $0x10, %eax\n"
        //"mov %eax, %ds\n"
        //"mov %eax, %es\n"
        //"mov %eax, %fs\n"
        //"mov %eax, %gs\n"

        // "mov $0xB, %eax\n"
        // "mov %eax, %cs\n"
        // "mov $0x90000, %esp\n"
        

        //"mov $1, %eax\n"

	    "movl $4, %edi\n"				// move value 4 in edx
	    "mul %edi\n"					// eax = eax * edx
	    "add $syscalls, %eax\n"			// add offset in eax to the beginning of the
	    "call *(%eax)\n"				// syscalls array to get the right syscall
	    "add $4, %esp\n"

	    "pop %ebx\n"
	    "pop %ecx\n"
	    "pop %edx\n"
	    "pop %esi\n"
	    "pop %edi\n"
	    "pop %ds\n"
	    "pop %es\n"
	    "pop %fs\n"
	    "pop %gs\n"
	    "add $4, %esp\n"
	    "syscall_invalid:\n"
	    "iret");
}

#endif /* !KERNEL_SYSCALL_H */
