#include <arch/i386/gdt.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <string.h>

tss_entry_t kernel_context;
static uint32_t next_available_task_id = 2;
extern task_struct *current_running_task;

void get_instruction_pointer(uint32_t *ip) {
    __asm__ __volatile__ ("movl $., %%edx": "=d"(*ip));
}

void get_idt_address(uint32_t *idt_addr) {
    __asm__ __volatile__ ("sidt %0" : "=m"(*idt_addr));
}

// create task_struct for the given task
task_struct *create_task(void *exec_address, int argc, char **argv, int userspace) {
    task_struct *task = (task_struct*) kmalloc(sizeof(task_struct));

    if (task != NULL) {
        task->state = TASK_CREATED;
        task->task_id = next_available_task_id++;
        task->exec_address = exec_address;
        task->argc = argc;

        if (argv != NULL) {
            task->argv = (char**) kmalloc(sizeof(char*) * argc);

            if (task->argv == NULL) {
                kfree(task);
                return NULL;
            }
        }

        for (int i = 0; i < argc; i++) {
            task->argv[i] = (char*) kmalloc(sizeof(char) * 10); // max 10 characters

            if (task->argv[i] == NULL) {
                kfree(task);
                return NULL;
            }

            strcpy(task->argv[i], argv[i]);
        }

        if (userspace) {
            task->vas = create_address_space();
        }
        else {
            task->vas = NULL;
        }

        task->context = NULL;
    }

    return task;
}

// free memory
void destroy_task(task_struct *task) {
    kfree(task->context);

    for (int i = 0; i < task->argc; i++) {
        kfree(task->argv[i]);
    }
    kfree(task->argv);

    kfree(task);
}

void enter_usermode(uint32_t entry_point, uint32_t stack_address) {
    
    // save current context somehow to restore it when exit is called?
    // maybe update the TSS
    //__asm__ __volatile__ ("mov %%esp, %%edx" : "=d"(kernel_context.esp));
    //__asm__ __volatile__ ("mov %%ebp, %%edx" : "=d"(kernel_context.ebp));
    //__asm__ __volatile__ ("pushf\n" "pop %%edx" : "=d"(kernel_context.eflags));

    __asm__ __volatile__ ("sti\n"               // otherwise, interrupt were disabled during
                                                // program execution
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


//__attribute__ ((naked)) void enter_usermode_resume_context(void) {
//    
//    __asm__ __volatile__ ("sti\n"
//
//                          "mov $0x23, %%eax\n"  // set segments to user mode data segment selector
//                          "mov %%eax, %%ds\n"
//                          "mov %%eax, %%es\n"
//                          "mov %%eax, %%fs\n"
//                          "mov %%eax, %%gs\n"
//
//                          // restore general registers
//                          "mov %0, %%eax\n"
//                          "mov %1, %%ebx\n"
//                          "mov %2, %%ecx\n"
//                          "mov %3, %%edx\n"
//
//                          "push $0x23\n"        // 0x23 is the user mode data segment selector
//                          "push %4\n"           // push the user stack address
//                          "pushf\n"             // push eflags register
//                          "push $0x1b\n"        // 0x1b is the user mode code segment selector
//                          "push %5\n"           // push the instruction pointer
//
//                          "iret\n"              // perform iret
//                          : : "r"(current_running_task->context->eax),
//                              "r"(current_running_task->context->ebx),
//                              "r"(current_running_task->context->ecx),
//                              "r"(current_running_task->context->edx),
//                              "r"(current_running_task->context->esp),
//                              "r"(current_running_task->context->eip));
//}

void enter_usermode_resume_context(void) {
    
    printk("esp: %x\n", current_running_task->context->esp);
    if (current_running_task->context->eip == 0 ||
            current_running_task->context->esp == 0 ||
            current_running_task->context->ebp == 0) {
        printk("strange context, returning...\n");

        // schedule();
        // return; -> same problem as in the case of ./file_does_not_exits situation
    }

    __asm__ __volatile__ ("sti\n"
                          "mov $0x23, %eax\n"
                          "mov %eax, %ds\n"
                          "mov %eax, %es\n"
                          "mov %eax, %fs\n"
                          "mov %eax, %gs\n");

    // restore general registers
    __asm__ __volatile__ ("mov %%eax, %%ebx" : : "a"(current_running_task->context->eax));
    __asm__ __volatile__ ("mov %%eax, %%ecx" : : "a"(current_running_task->context->ecx));
    __asm__ __volatile__ ("mov %%eax, %%edx" : : "a"(current_running_task->context->edx));
    __asm__ __volatile__ ("mov %%eax, %%edi" : : "a"(current_running_task->context->edi));
    __asm__ __volatile__ ("mov %%eax, %%esi" : : "a"(current_running_task->context->esi));
    __asm__ __volatile__ ("mov %%eax, %%ebp" : : "a"(current_running_task->context->ebp));

    // restore eax
    __asm__ __volatile__ ("push %ebx");
    __asm__ __volatile__ ("mov %%ebx, %%eax\n"
                          "pop %%ebx" : : "b"(current_running_task->context->ebx));

    // restore flags
    __asm__ __volatile__ ("push %%eax\n"
                          "popf\n" : : "a"(current_running_task->context->flags));

    __asm__ __volatile__ ("push $0x23\n"
                          "push %%eax\n"
                          "pushf\n"
                          "push $0x1b\n" :: "a"(current_running_task->context->esp));
    __asm__ __volatile__ ("push %%eax" :: "a"(current_running_task->context->eip));

    __asm__ __volatile__ ("iret");
}

//void enter_usermode_resume_context(void) {
//    
//    __asm__ __volatile__ ("sti\n"               // otherwise, interrupt were disabled during
//                                                // program execution
//
//                          "mov $0x23, %%eax\n"  // set segments to user mode data segment selector
//                          "mov %%eax, %%ds\n"
//                          "mov %%eax, %%es\n"
//                          "mov %%eax, %%fs\n"
//                          "mov %%eax, %%gs\n"
//
//                          "push $0x23\n"        // 0x23 is the user mode data segment selector
//                          "push %%ebx\n"        // push the user stack address
//                          "pushf\n"             // push eflags register
//                          "push $0x1b\n"        // 0x1b is the user mode code segment selector
//                          "push %1\n"           // push the instruction pointer
//
//                          "iret\n"              // perform iret
//                          : : "b"(current_running_task->context->esp), "r"(current_running_task->context->eip));
//}

