#ifndef _SCH_H
#define _SCH_H 1

#include <process/process.h>
#include <arch/i386/isr.h>
#include <stdint.h>

// node in the task queue
struct task_node {
    struct task_struct *task;
    struct task_node *next;
};

// task queue
struct task_queue {
    struct task_node *front;
    struct task_node *rear;
};

typedef enum {
	RUNNING_TASK_QUEUE,
	SLEEPING_TASK_QUEUE
} QUEUE_TYPE;

void enqueue_task(struct task_struct *, QUEUE_TYPE);
struct task_struct *dequeue_task(QUEUE_TYPE);

#ifdef CONFIG_FCFS_SCH
uint8_t scheduler_init(void);
uint8_t init_task_queue(void);
void simple_task_scheduler(void);
#else
uint8_t scheduler_init_rr(void);
uint32_t queue_size(QUEUE_TYPE);
void schedule(QUEUE_TYPE);
void start_init_task(void);
void change_context(struct interrupt_regs *);
void resume_context(struct interrupt_regs *);
void change_context_kernel(struct interrupt_regs *);
void display_running_processes(void);
void save_current_context(struct interrupt_regs *r);
#endif

#endif /* !_SCH_H */

