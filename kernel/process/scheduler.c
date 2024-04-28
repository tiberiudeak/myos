#include <arch/i386/isr.h>
#include <process/process.h>
#include <process/scheduler.h>
#include <arch/i386/pit.h>
#include <kernel/tty.h>
#include <kernel/shell.h>
#include <mm/kmalloc.h>
#include <mm/vmm.h>
#include <stddef.h>

task_queue_t *task_queue = NULL;
task_struct *current_running_task;
uint8_t scheduler_initialized = 0;

#ifdef CONFIG_RR_TIME_QUANTUM
const uint32_t running_time_quantum_ms = CONFIG_RR_TIME_QUANTUM;
#endif

/**
 * @brief Put task in task queue
 *
 * This function puts the given task in the task queue.
 *
 * @param task  The task to be put in the task queue
 */
void enqueue_task(task_struct *task) {
    task_node_t *new_node = kmalloc(sizeof(task_node_t));

    if (new_node != NULL) {
        new_node->task = task;
        new_node->next = NULL;

        if (task_queue->rear == NULL) {
            task_queue->front = new_node;
        }
        else {
            task_queue->rear->next = new_node;
        }

        task_queue->rear = new_node;
    }
}

/**
 * @brief Get task from the task queue
 *
 * This fuction takes out and returns a task from the task queue if available.
 *
 * @return The task taken out from the queue
 */
task_struct *dequeue_task(void) {
    if (task_queue->front == NULL)
        return NULL;    // empty queue


    task_node_t *front_node = task_queue->front;

    task_struct *task = front_node->task;
    task_queue->front = front_node->next;

    if (task_queue->front == NULL) {
        task_queue->rear = NULL;    // empty queue
    }

    kfree(front_node);
    return task;
}

#ifdef CONFIG_SIMPLE_SCH
/**
 * @brief Initialize the scheduler task queue
 *
 * This function calls the init_task_queue() function
 * to initialize the task queue. Only used as a wrapper
 * for the other function.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t scheduler_init(void) {
    return init_task_queue();
}

/**
 * @brief Initialize the scheduler task queue
 *
 * This function performs the actual initialization of the task
 * queue.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t init_task_queue(void) {
    task_queue = kmalloc(sizeof(task_queue_t));

    if (task_queue == NULL) {
        printk("out of memory\n");
        return 1;
    }

    task_queue->front = NULL;
    task_queue->rear = NULL;

    scheduler_initialized = 1;

    return 0;
}

/**
 * @brief Check task queue and execute tasks if available
 *
 * This function checks if there are available tasks in the task queue. If
 * present, the task is taken out from the queue and executed. If none are
 * present, then the scheduler will sleep for some time and then look again
 * for tasks in the queue.
 *
 */
void simple_task_scheduler(void) {
    while (1) {
        if (task_queue->front == NULL) {
            // sleep a bit 
            wait_millis(100);

            continue;
        }

        task_struct *task = dequeue_task();
        current_running_task = task;

        printk("executing task: %d\n", task->task_id);

        void (*program)(int argc, char *argv[]);
        program = (void (*)(int, char**)) task->exec_address;

        // change virtual address space
        if (task->vas != NULL) {
            printk("setting new vas\n");
            set_page_directory(task->vas);
        }

        // execute task
        program(task->argc, task->argv);

        // following code is executed only if the program could not be executed for
        // some reason (file not present, not an ELF file, etc...)
        destroy_task(task);
        shell_cleanup();
    }
}
#else

/**
 * @brief Get size of the task queue
 *
 * This function returns the size of the task queue.
 *
 * @return The queue size
 */
uint32_t queue_size(void) {
    uint32_t count = 0;

    task_node_t *current = task_queue->front;
    while (current != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

/**
 * @ Init task
 *
 * This function represents the first task created and executed.
 *
 * @param argc Number of arguments
 * @param argv Arguments
 */
void init_task_func(int argc, char *argv[]) {
    printk("init process started!\n");
    while (1) __asm__ __volatile__ ("sti; hlt; cli");
}

/**
 * @brief Initialize the round-robin scheduler's task queue
 *
 * This function performs the actual initialization of the task
 * queue by creating and adding to the queue the init task.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t init_task_queue_rr(void) {
    task_queue = kmalloc(sizeof(task_queue_t));

    if (task_queue == NULL) {
        printk("out of memory\n");
        return 1;
    }

    task_queue->front = NULL;
    task_queue->rear = NULL;

    // create init task and add it to the queue
    void (*init)(int, char**) = init_task_func;
    task_struct *init_task = create_task(init, 0, NULL, 0);

    if (init_task == NULL) {
        printk("init task is NULL!\n");
        return 1;
    }

    enqueue_task(init_task);
    
    scheduler_initialized = 1;

    return 0;
}

/**
 * @brief Start init task
 *
 * This function takes a task from the task queue (when this function
 * is called, the only task in the task queue will be init), updates
 * the current running task and starts its execution.
 */
void start_init_task(void) {
    // take init task from the queue
    task_struct *task = dequeue_task();

    // update current running task
    current_running_task = task;

    // start execution of init
    void (*program)(int argc, char *argv[]);
    program = (void (*)(int, char**)) task->exec_address;
    program(task->argc, task->argv);
}

/**
 * @brief Initialize the round-robin scheduler
 *
 * This function calls the init_task_queue_rr() function
 * to initialize the task queue. Only used as a wrapper
 * for the other function.
 *
 * @return 1 if error occured, 0 otherwise
 */
uint8_t scheduler_init_rr(void) {
    return init_task_queue_rr();
}

/**
 * @brief Change contex for a user space task in the TASK_CREATED
 *        state (described only by an instruction pointer and stack)
 *
 * -- only called for user space tasks without prior context --
 *
 * This function changes the context for a user space task
 * described only by its instruction pointer and stack.
 *
 * @param r "Context" of the previous task that needs to
 *          be updated
 */
void change_context(interrupt_regs *r) {
    // TODO: update TSS for ring 0

    __asm__ __volatile__ ("mov $0x23, %eax\n"
                          "mov %eax, %ds\n"
                          "mov %eax, %es\n"
                          "mov %eax, %fs\n"
                          "mov %eax, %gs");

    // change DS and SS to user mode data segment selector
    *(&r->ds) = 0x23;
    *(&r->ss) = 0x23;

    // change stack address
    *(&r->useresp) = current_running_task->context->esp;

    // clear flags (except for the interrupt bit)
    // TODO

    // change CS to user mode code segment selector
    *(&r->cs) = 0x1B;

    // change instruction pointer
    *(&r->eip) = current_running_task->context->eip;
}

/**
 * @brief Switch context
 *
 * This functions switches the context of the previous running
 * task with the context of another task (that has become the
 * current running task).
 *
 * @param r "Context" of the previous running task that needs to
 *          be updated
 */
void resume_context(interrupt_regs *r) {
    // restore segments
    *(&r->ss) = current_running_task->context->ss;
    *(&r->ds) = current_running_task->context->ds;
    *(&r->cs) = current_running_task->context->cs;

    // restore general registers
    *(&r->eax) = current_running_task->context->eax;
    *(&r->ebx) = current_running_task->context->ebx;
    *(&r->ecx) = current_running_task->context->ecx;
    *(&r->edx) = current_running_task->context->edx;
    *(&r->edi) = current_running_task->context->edi;
    *(&r->esi) = current_running_task->context->esi;

    // restore flags
    *(&r->eflags) = current_running_task->context->flags;

    // restore stack
    *(&r->useresp) = current_running_task->context->esp;
    *(&r->ebp) = current_running_task->context->ebp;

    // restore instruction pointer
    *(&r->eip) = current_running_task->context->eip;
}

/**
 * @brief Round-Robin Scheduler
 *
 * This function puts the current running task in the queue (if the
 * task is not terminated), takes a task from the queue and updates
 * the current running task with the new task. The function also
 * changes the virtual address space if the new task runs in user space.
 */
void schedule(void) {

    // put current running task in the queue if task
    // if not terminated
    if (current_running_task != NULL) {
        enqueue_task(current_running_task);
    }

    // take task from the queue and update current running task
    task_struct *task = dequeue_task();
    current_running_task = task;

    // change virtual address space for user tasks
    if (task->vas != NULL) {
        set_page_directory(task->vas);
    }
}

#endif

