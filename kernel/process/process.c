#include <arch/i386/gdt.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <process/process.h>
#include <string.h>

tss_entry_t kernel_context;
static uint32_t next_available_task_id = 2;

void get_instruction_pointer(uint32_t *ip) {
    __asm__ __volatile__ ("movl $., %%edx": "=d"(*ip));
}

void get_idt_address(uint32_t *idt_addr) {
    __asm__ __volatile__ ("sidt %0" : "=m"(*idt_addr));
}

// create task_struct for the given task
task_struct *create_task(void *exec_address, int argc, char **argv) {
    task_struct *task = (task_struct*) kmalloc(sizeof(task_struct));

    if (task != NULL) {
        task->state = TASK_CREATED;
        task->task_id = next_available_task_id++;
        task->exec_address = exec_address;
        task->argc = argc;

        task->argv = (char**) kmalloc(sizeof(char*) * argc);

        if (task->argv == NULL) {
            kfree(task);
            return NULL;
        }

        for (int i = 0; i < argc; i++) {
            task->argv[i] = (char*) kmalloc(sizeof(char) * 10); // max 10 characters

            if (task->argv[i] == NULL) {
                kfree(task);
                return NULL;
            }

            strcpy(task->argv[i], argv[i]);
        }
    }

    return task;
}

// free memory
void destroy_task(task_struct *task) {
    for (int i = 0; i < task->argc; i++) {
        kfree(task->argv[i]);
    }
    kfree(task->argv);

    kfree(task);
}

void enter_usermode(uint32_t entry_point, uint32_t stack_address) {
    
    // save current context somehow to restore it when exit is called?
    // maybe update the TSS
    __asm__ __volatile__ ("mov %%esp, %%edx" : "=d"(kernel_context.esp));
    __asm__ __volatile__ ("mov %%ebp, %%edx" : "=d"(kernel_context.ebp));
    __asm__ __volatile__ ("pushf\n" "pop %%edx" : "=d"(kernel_context.eflags));

    __asm__ __volatile__ ("cli\n"
                          "mov $0x23, %%eax\n"  // set segments to user mode data segment selector
                          "mov %%eax, %%ds\n"
                          "mov %%eax, %%es\n"
                          "mov %%eax, %%fs\n"
                          "mov %%eax, %%gs\n"

                          "push $0x23\n"        // 0x23 is the user mode data segment selector
                          "push %%ebx\n"        // push the user stack address
                          "pushf\n"             // push eflags register
                          "push $0x1b\n"        // 0x1b is the user mode code segment selector
                          "push %1\n"           // push the instruction pointer

                          "iret\n"              // perform iret
                          "user_finish_exec:"
                          : : "b"(stack_address), "r"(entry_point));
}

