#include <process/process.h>
#include <process/scheduler.h>
#include <arch/i386/gdt.h>
#include <kernel/tty.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <string.h>

static uint32_t next_available_task_id = 1;

/**
 * @brief Create a task
 *
 * This function created a new task based on the given parameters.
 *
 * @param exec_address The instruction pointer to the beginning of the task (only
 *                      used for the simple scheduler)
 * @param argc         The arguments conunt
 * @param argv         The values for the arguments
 * @userspace          Set to 1 if task will run in userspace, 0 otherwise
 *
 * @return The created task
 */
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
                // free previous allocated argvs
                for (int j = 0; j < i; j++) {
                    kfree(task->argv[j]);
                }

                kfree(task->argv);
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

        task->context = kmalloc(sizeof(proc_context_t));

        if (task->context == NULL) {
            // free previous allocated memory
            if (argc > 0) {
                for (int i = 0; i < argc; i++) {
                    kfree(task->argv[i]);
                }

                kfree(task->argv);
                kfree(task);
            }

            return NULL;
        }
    }

    return task;
}

/**
 * @brief Free memory for the given task from the kernel heap
 *
 * This function frees all memory allocated on the kernel heap used
 * for the given task.
 *
 * @param task The task
 */
void destroy_task(task_struct *task) {
    for (int i = 0; i < task->argc; i++) {
        kfree(task->argv[i]);
    }

    kfree(task->argv);
    kfree(task->context);
    kfree(task);
}

