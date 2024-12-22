#ifndef _SCH_H
#define _SCH_H 1

#include <process/process.h>
#include <arch/i386/isr.h>
#include <kernel/list.h>
#include <stdint.h>

// node in the task queue
struct task_node {
    struct task_struct *task;
    struct embedded_link list;
};

/*
 * delta queue for sleeping tasks
 *
 * tasks are sorted by the difference in sleeping
 * times relative to the previous task
 *
 * e.g. task1 sleeps 10ms, task2 12ms and task3 2ms:
 *
 *          -------    -------    -------
 * head ->  | 2ms | -> | 8ms | -> | 2ms |
 *          -------    -------    -------
 *           task3      task1      task2
 *
 * the head represents the next task to wake up, with the
 * smallest remaining time
 */
struct delta_queue_node {
	int delta_time_ms;
	struct task_struct *task;
	struct embedded_link list;
};

typedef enum {
	RUNNING_TASK_QUEUE,
	SLEEPING_TASK_QUEUE
} QUEUE_TYPE;

void enqueue_task(struct task_struct *);
struct task_struct *dequeue_task(void);

#ifdef CONFIG_FCFS_SCH
uint8_t scheduler_init(void);
uint8_t init_task_queue(void);
void simple_task_scheduler(void);
#else
uint8_t scheduler_init_rr(void);
uint32_t queue_size(struct embedded_link *);
void schedule(QUEUE_TYPE);
void start_init_task(void);
void change_context(struct interrupt_regs *);
void resume_context(struct interrupt_regs *);
void change_context_kernel(struct interrupt_regs *);
void display_running_processes(void);
void save_current_context(struct interrupt_regs *r);
void dq_decrement_head(struct embedded_link *);
struct task_struct *dq_dequeue(struct embedded_link *);
void dq_enqueue(struct embedded_link *, struct task_struct *);
#endif

#endif /* !_SCH_H */

