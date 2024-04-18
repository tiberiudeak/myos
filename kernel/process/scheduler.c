#include <process/process.h>
#include <process/scheduler.h>
#include <arch/i386/pit.h>
#include <kernel/tty.h>
#include <kernel/shell.h>
#include <mm/kmalloc.h>
#include <stddef.h>

task_queue_t *task_queue = NULL;
task_struct *current_running_task;

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

    return 0;
}

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

        // execute task
        program(task->argc, task->argv);

        // following code is executed only if the program could not be executed for
        // some reason (file not present, not an ELF file, etc...)
        destroy_task(task);
        shell_cleanup();
    }
}
