#ifndef _SCH_H
#define _SCH_H 1

#include <process/process.h>
#include <stdint.h>

// node in the task queue
typedef struct task_node {
    task_struct *task;
    struct task_node *next;
} task_node_t;

// task queue
typedef struct {
    task_node_t *front;
    task_node_t *rear;
} task_queue_t;

uint8_t scheduler_init(void);
uint8_t init_task_queue(void);
void enqueue_task(task_struct *task);
task_struct *dequeue_task(void);
void simple_task_scheduler(void);

#endif /* !_SCH_H */

